// MSVC x86 / x86-64 only
#include <stdio.h>
#include <intrin.h>
#include <string.h>

int main()
{
// Get Basic Info
    printf("__Basic Info__:\n");

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

    // Get CPU Family / Model / Stepping etc.
    // 
    // fn_id 0x01: cpuInfo[0] binary 
    // 31-28 Rsv, 27-20 Ext. Family, 19-16 Ext. Model, 15-14 Rsv, 13-12 OEM Type
    // 11-08 Family, 07-04 Model, 03-00 Stepping

    __cpuid(cpuInfo, 0x01);
    // printf("%x %x %x %x\n", cpuInfo[0], cpuInfo[1], cpuInfo[2], cpuInfo[3]);

    int eax01 = cpuInfo[0];
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
    printf("\n__Cache__:\n");

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


    return 0;
}