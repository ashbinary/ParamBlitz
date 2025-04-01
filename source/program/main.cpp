#include "lib.hpp"
#include "loggers.hpp"
#include <nn.hpp>
#include <stdio.h>
#include <iostream>

nn::fs::FileHandle hashList;
s64 position = 0;

namespace sead {
    template<typename T>
    struct SafeStringBase {
        const T* mStringTop;

        virtual ~SafeStringBase() = default;
        virtual SafeStringBase& operator=(const SafeStringBase& other);
        virtual void assureTerminationImpl_() const {}
                
        SafeStringBase() : mStringTop("") {}
        SafeStringBase(const T* str) : mStringTop(str) {}
    };
    template<typename T>
    struct BufferedSafeStringBase : public SafeStringBase<T> {
        uint mLength;

        BufferedSafeStringBase(T* buffer, size_t size) : SafeStringBase<T>(buffer), mLength(size) {}
        BufferedSafeStringBase(const BufferedSafeStringBase&) = default;
        ~BufferedSafeStringBase() override = default;
        BufferedSafeStringBase<T>& operator=(const SafeStringBase<T>& other) override;
    };
    using SafeString = SafeStringBase<char>;
    using BufferedSafeString = BufferedSafeStringBase<char>;

    template <typename T, size_t Size>
    class FixedSafeStringBase : public BufferedSafeStringBase<T>
    {
        public:
            FixedSafeStringBase() : BufferedSafeStringBase<T>(mBuffer, Size) {}
            ~FixedSafeStringBase() override = default;
            FixedSafeStringBase& operator=(const SafeStringBase<T>& other) override;

        T mBuffer[Size];
    };

    template <size_t Size>
    class FixedSafeString : public FixedSafeStringBase<char, Size>
    {
        public:
            FixedSafeString() : FixedSafeStringBase<char, Size>() {}
            FixedSafeString(const SafeString& str) : FixedSafeStringBase<char, Size>(str) {}

            FixedSafeString<Size>& operator=(const SafeStringBase<char>& other) override;
    };
}

namespace Lp {
    namespace Sys {
        struct ParamNode {
            u64 field_8;
            const char *mParamName;
            uintptr_t *mNext;
            uintptr_t *mParent;
            u32 mFlags;
            u32 field_2C;

            uint keyHash() const;  
            virtual ~ParamNode();
            virtual void key(sead::BufferedSafeString*) const;
        };
    };
}

HOOK_DEFINE_INLINE(SaveBuffer) {
    static void Callback(exl::hook::InlineCtx* context)
    {
        sead::FixedSafeString<256>* bufferPtr = reinterpret_cast<sead::FixedSafeString<256>*>(context->X[0]);
        const size_t strSize = strlen(bufferPtr->mStringTop);

        nn::fs::WriteFile(hashList, position, bufferPtr->mStringTop, strSize, 
            nn::fs::WriteOption::CreateOption(nn::fs::WriteOptionFlag_Flush));
        position += strSize;

        nn::fs::WriteFile(hashList, position, "\n", strlen("\n"), 
            nn::fs::WriteOption::CreateOption(nn::fs::WriteOptionFlag_Flush));
        position += strlen("\n");
    };
};

extern "C" void exl_main(void* x0, void* x1) {
    /* Setup hooking environment. */
    exl::hook::Initialize();

    nn::fs::MountSdCardForDebug("sd");
    nn::fs::CreateFile("sd:/bprmList.txt", 0);
    nn::fs::OpenFile(&hashList, "sd:/bprmList.txt", nn::fs::OpenMode_ReadWrite | nn::fs::OpenMode_Append);

    /* Install the hook at the provided function pointer. Function type is checked against the callback function. */
    //GetParamData::InstallAtFuncPtr(&Lp::Sys::ParamNode::keyHash);
    SaveBuffer::InstallAtOffset(0x019D1000); // At hashing function

    /* Alternative install funcs: */
    /* InstallAtPtr takes an absolute address as a uintptr_t. */
    /* InstallAtOffset takes an offset into the main module. */

    /*
    For sysmodules/applets, you have to call the entrypoint when ready
    exl::hook::CallTargetEntrypoint(x0, x1);
    */
}

extern "C" NORETURN void exl_exception_entry() {
    /* Note: this is only applicable in the context of applets/sysmodules. */
    EXL_ABORT("Default exception handler called!");
}