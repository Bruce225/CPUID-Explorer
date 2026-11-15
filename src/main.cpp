// Windows, MSVC x86 / x86-64 only
#include <stdio.h>
#include <intrin.h>
#include <string.h>
#include <stdint.h>
#include <windows.h>

int main()
{
// Get Basic Info
    printf("__ Basic Info __:\n");

    int cpuInfo[4] = {0};
    __cpuid(cpuInfo, 0);
    // printf("%x %x %x %x\n", cpuInfo[0], cpuInfo[1], cpuInfo[2], cpuInfo[3]);

    // Get CPU Vendor, e.g. AuthenticAMD
    char cpuVendor[13];
    memcpy(cpuVendor, cpuInfo + 1, 4);
    memcpy(cpuVendor + 4, cpuInfo + 3, 4);  // EDX prior to ECX
    memcpy(cpuVendor + 8, cpuInfo + 2, 4);
    cpuVendor[12] = '\0';
    if (strcmp(cpuVendor, "GenuineIntel") == 0)
        printf("CPU Vendor: Intel\n");
    else if (strcmp(cpuVendor, "AuthenticAMD") == 0)
        printf("CPU Vendor: AMD\n");
    else printf("CPU Vendor: Unknown CPU Vendor\n");

    // Get Max Basic Fn
    printf("Max Basic Function: 0x%x\n", cpuInfo[0]); 

    // Get Max Ext. Fn
    __cpuid(cpuInfo, 0x80000000);
    printf("Max Extended Function: 0x%x\n", cpuInfo[0]);

    // Get CPU Brand, e.g. Intel(R) Core(TM) ... 
    char cpuBrand[49];
    for (int i = 0; i <= 2; i++)
    {
        __cpuid(cpuInfo, 0x80000002 + i);
        memcpy(cpuBrand + (i * 16), cpuInfo, 16);
    }
    cpuBrand[48] = '\0';
    printf("CPU Brand: %s\n", cpuBrand);

    // Get CPU Cores & Threads (Suitable for Non-hybrid architecture CPUs)
    __cpuidex(cpuInfo, 0x0B, 0);
    int numDfLogiProcPerCore = cpuInfo[1] & 0xFFFF;
    __cpuidex(cpuInfo, 0x0B, 1);
    int numDfLogiCores = cpuInfo[1] & 0xFFFF;

    int activeThreads = GetActiveProcessorCount(ALL_PROCESSOR_GROUPS);

    unsigned char buffer[4097];
    DWORD size = sizeof(buffer);
    GetLogicalProcessorInformationEx(
        RelationProcessorCore, (PSYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX)buffer, &size);
    int isSMTActive =
        ((((PSYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX)buffer)->Processor.Flags) & LTP_PC_SMT) != 0;

    printf("Hyper-Threading: %sSupported, currently %s\n", (numDfLogiProcPerCore > 1) ? "" : "Not ",
        isSMTActive ? "ON" : "OFF");
    printf("Active: %d / %d Cores, %d / %d Threads\n", activeThreads / (isSMTActive + 1),
        numDfLogiCores / numDfLogiProcPerCore, activeThreads ,numDfLogiCores);

    // Get CPU Family / Model / Stepping etc.
    // 
    // fn_id 0x01: cpuInfo[0] binary 
    // 31-28 Rsv, 27-20 Ext. Family, 19-16 Ext. Model, 15-14 Rsv, 13-12 OEM Type
    // 11-08 Family, 07-04 Model, 03-00 Stepping

    __cpuid(cpuInfo, 0x01);
    // printf("%x %x %x %x\n", cpuInfo[0], cpuInfo[1], cpuInfo[2], cpuInfo[3]);

    int eax01 = cpuInfo[0], ecx01 = cpuInfo[2], edx01 = cpuInfo[3];
    int stepping = eax01 & 0xF, model = (eax01 >> 4) & 0xF, family = (eax01 >> 8) & 0xF;
    int procType = (eax01 >> 12) & 0x3;
    int extModel = (eax01 >> 16) & 0xF, extFamily = (eax01 >> 20) & 0xFF;

    int displayFamily = family + ((family == 0xF) ? extFamily : 0);
    int displayModel = model + (((family == 0x6) || (family == 0xF)) ? (extModel << 4) : 0);

    const char *const procTypeName[4] = { "OEM", "Intel OverDrive(R)", "Dual Processor", "Reserved" };

    printf("Procseeor Type: %s\n", procTypeName[procType]);
    printf("Family: %x  Model: %x  Stepping: %x\n", family, model, stepping);
    printf("Extended Family: %x  Extended Model: %x\n", displayFamily, displayModel);
    
// Get Cache
    printf("\n__ Cache __:\n");

    for (int i = 0; i <= 31; i++) 
    {
        __cpuidex(cpuInfo, 0x04, i);

        int eax04 = cpuInfo[0];
        int cacheType = eax04 & 0x1F, cacheLevel = (eax04 >> 5) & 0x7;
        int isSelfIni = (eax04 >> 8) & 0x1, isFullyAssoc = (eax04 >> 9) & 0x1;
        int maxLogiProcSharing = ((eax04 >> 14) & 0xFFF) + 1, 
            maxCoresPhysPkg = ((eax04 >> 26) & 0x3F) + 1;

        if (cacheType == 0) break;

        int ebx04 = cpuInfo[1], ecx04 = cpuInfo[2];
        int sysCohLnSize = (ebx04 & 0xFFF) + 1, physLnPart = ((ebx04 >> 12) & 0x3FF) + 1,
            waysOfAssoc = ((ebx04 >> 22) & 0x3FF) + 1;
        int numSets = ecx04 + 1;

        int cacheSize = sysCohLnSize * physLnPart * waysOfAssoc * numSets;

        int edx04 = cpuInfo[3];
        int isInclusive = (edx04 >> 1) & 0x1;

        printf("L%x%s Cache:\n", cacheLevel, (cacheType == 1) ? "D" : ((cacheType == 2) ? "I" : ""));
        printf("Size = %g %s\n",
            ((cacheSize / 1024) >= 1024) ? (cacheSize * 1.0 / (1024 * 1024)) : (cacheSize * 1.0 / 1024),
            ((cacheSize / 1024) >= 1024) ? "MB" : "KB");
        printf("Ways = %d Ways\n", waysOfAssoc);
        printf("%sInclusive\n", isInclusive ? "" : "Not ");
        //printf("%x %x %x %x %d %d\n", cacheType, cacheLevel, isSelfIni, isFullyAssoc, 
        //    maxLogiProcSharing, maxCoresPhysPkg);

        printf("\n");
    }


// Get Instruction Sets Supported
    printf("__ Supported Instruction Sets__\n");

    __cpuidex(cpuInfo, 0x07, 0);
    int ebx07_0 = cpuInfo[1], ecx07_0 = cpuInfo[2], edx07_0 = cpuInfo[3];
    __cpuidex(cpuInfo, 0x07, 1);
    int eax07_1 = cpuInfo[0];

    typedef struct
    {
        uint32_t EM64T      : 1;
        uint32_t VMX        : 1;
        uint32_t TSX        : 1;
        uint32_t SGX        : 1;

        uint32_t MMX        : 1;
        uint32_t SSE        : 1;
        uint32_t SSE2       : 1;
        uint32_t SSE3       : 1;
        uint32_t SSSE3      : 1;
        uint32_t SSE4_1     : 1;
        uint32_t SSE4_2     : 1;

        uint32_t AVX        : 1;
        uint32_t AVX2       : 1;
        uint32_t FMA3       : 1;
        uint32_t F16C       : 1;
        uint32_t BMI1       : 1;
        uint32_t BMI2       : 1;
        uint32_t POPCNT     : 1;
        uint32_t MOVBE      : 1;
        uint32_t ADX        : 1;
        uint32_t AES_NI     : 1;
        uint32_t PCLMULQDQ  : 1;
        uint32_t SHA        : 1;
        uint32_t RDRAND     : 1;

        uint32_t AVXVNNI    : 1;
        uint32_t AMX        : 1;
        
        uint32_t AVX512F    : 1;
        uint32_t AVX512DQ   : 1;
        uint32_t AVX512CD   : 1;
        uint32_t AVX512BW   : 1;
        uint32_t AVX512VL   : 1;
        uint32_t AVX512VNNI : 1;
    } cpuInsSets;

    cpuInsSets sup = {0};

    sup.MMX         = (edx01 >> 23) & 0x1;
    sup.SSE         = (edx01 >> 25) & 0x1;
    sup.SSE2        = (edx01 >> 26) & 0x1;
    sup.SSE3        = ecx01 & 0x1;
    sup.SSSE3       = (ecx01 >> 9) & 0x1;
    sup.SSE4_1      = (ecx01 >> 19) & 0x1;
    sup.SSE4_2      = (ecx01 >> 20) & 0x1;

    sup.AVX         = (ecx01 >> 28) & 0x1;
    sup.AVX2        = (ebx07_0 >> 5) & 0x1;
    sup.FMA3        = (ecx01 >> 12) & 0x1;
    sup.F16C        = (ecx01 >> 29) & 0x1;
    sup.BMI1        = (ebx07_0 >> 3) & 0x1;
    sup.BMI2        = (ebx07_0 >> 8) & 0x1;
    sup.POPCNT      = (ecx01 >> 23) & 0x1;
    sup.MOVBE       = (ecx01 >> 22) & 0x1;
    sup.ADX         = (ebx07_0 >> 19) & 0x1;
    sup.AES_NI      = (ecx01 >> 25) & 0x1;
    sup.PCLMULQDQ   = (ecx01 >> 1) & 0x1;
    sup.SHA         = (ebx07_0 >> 29) & 0x1;
    sup.RDRAND      = (ecx01 >> 30) & 0x1;

    sup.AVXVNNI     = (eax07_1 >> 4) & 0x1;
    sup.AMX         = (edx07_0 >> 22) & 0x1;
    
    sup.AVX512F     = (ebx07_0 >> 16) & 0x1;
    sup.AVX512DQ    = (ebx07_0 >> 17) & 0x1;
    sup.AVX512CD    = (ebx07_0 >> 28) & 0x1;
    sup.AVX512BW    = (ebx07_0 >> 30) & 0x1;
    sup.AVX512VL    = (ebx07_0 >> 31) & 0x1;
    sup.AVX512VNNI  = (ecx07_0 >> 11) & 0x1;

    printf("%12s %s\n", "MMX:", sup.MMX ? "Yes" : "No");
    printf("%12s %s\n", "SSE:", sup.SSE ? "Yes" : "No");
    printf("%12s %s\n", "SSE2:", sup.SSE2 ? "Yes" : "No");
    printf("%12s %s\n", "SSE3:", sup.SSE3 ? "Yes" : "No");
    printf("%12s %s\n", "SSSE3:", sup.SSSE3 ? "Yes" : "No");
    printf("%12s %s\n", "SSE4.1:", sup.SSE4_1 ? "Yes" : "No");
    printf("%12s %s\n", "SSE4.2:", sup.SSE4_2 ? "Yes" : "No");

    printf("%12s %s\n", "AVX:", sup.AVX ? "Yes" : "No");
    printf("%12s %s\n", "AVX2:", sup.AVX2 ? "Yes" : "No");
    printf("%12s %s\n", "FMA3:", sup.FMA3 ? "Yes" : "No");
    printf("%12s %s\n", "F16C:", sup.F16C ? "Yes" : "No");
    printf("%12s %s\n", "BMI1:", sup.BMI1 ? "Yes" : "No");
    printf("%12s %s\n", "BMI2:", sup.BMI2 ? "Yes" : "No");
    printf("%12s %s\n", "POPCNT:", sup.POPCNT ? "Yes" : "No");
    printf("%12s %s\n", "MOVBE:", sup.MOVBE ? "Yes" : "No");
    printf("%12s %s\n", "ADX:", sup.ADX ? "Yes" : "No");
    printf("%12s %s\n", "AES-NI:", sup.AES_NI ? "Yes" : "No");
    printf("%12s %s\n", "PCLMULQDQ:", sup.PCLMULQDQ ? "Yes" : "No");
    printf("%12s %s\n", "SHA:", sup.SHA ? "Yes" : "No");
    printf("%12s %s\n", "RDRAND:", sup.RDRAND ? "Yes" : "No");

    printf("%12s %s\n", "AVXVNNI:", sup.AVXVNNI ? "Yes" : "No");
    printf("%12s %s\n", "AMX:", sup.AMX ? "Yes" : "No");

    printf("%12s %s\n", "AVX512F:", sup.AVX512F ? "Yes" : "No");
    printf("%12s %s\n", "AVX512DQ:", sup.AVX512DQ ? "Yes" : "No");
    printf("%12s %s\n", "AVX512CD:", sup.AVX512CD ? "Yes" : "No");
    printf("%12s %s\n", "AVX512BW:", sup.AVX512BW ? "Yes" : "No");
    printf("%12s %s\n", "AVX512VL:", sup.AVX512VL ? "Yes" : "No");
    printf("%12s %s\n", "AVX512VNNI:", sup.AVX512VNNI ? "Yes" : "No");

    system("pause");
    return 0;
}