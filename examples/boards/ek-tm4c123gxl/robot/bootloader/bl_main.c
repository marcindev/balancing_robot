#include "config.h"
#include "driverlib/eeprom.h"
#include "driverlib/sysctl.h"
#include "inc/hw_nvic.h"
#include "inc/hw_types.h"

#define BINARY1_ADDRESS     0x4000
#define BINARY2_ADDRESS     0x1E000

#define RESET_ISR_OFFSET    0x4

bool initializeEEPROM()
{
    SysCtlPeripheralEnable(SYSCTL_PERIPH_EEPROM0);

    return EEPROMInit() == EEPROM_INIT_OK;
}

//__asm void boot_jump( uint32_t address )
//{
//   LDR SP, [R0]       ;Load new stack pointer address
//   LDR PC, [R0, #4]   ;Load new program counter address
//}
void boot_jump( uint32_t address )
{
    __asm("    ldr     sp, [r0]\n"
          "    ldr     pc, [r0, #4]");
}


int main()
{
    if(!initializeEEPROM()) while(true) { }

    bool isConfigChanged = false;
    uint32_t jumpAddress;
    void (*application)(void);

    readConfig();

    if(config.binary1.address != BINARY1_ADDRESS
            || config.binary2.address != BINARY2_ADDRESS)
    {
        config.binary1.address = BINARY1_ADDRESS;
        config.binary2.address = BINARY2_ADDRESS;
        isConfigChanged = true;
    }

    uint32_t address1 = config.binary1.address;
    uint32_t address2 = config.binary2.address;
    uint16_t version1 = config.binary1.version;
    uint16_t version2 = config.binary2.version;
    bool* isActive1 = &config.binary1.isActive;
    bool* isActive2 = &config.binary2.isActive;
    bool* isChecked1 = &config.binary1.isChecked;
    bool* isChecked2 = &config.binary2.isChecked;
    bool* isGood1 = &config.binary1.isGood;
    bool* isGood2 = &config.binary2.isGood;


    if(*isActive1)
    {
        jumpAddress = address1;

        if(version2 > version1)
        {
            if(!(*isChecked2))
            {
                jumpAddress = address2;
            }
            else if(*isGood2)
            {
                jumpAddress = address2;
                *isActive2 = true;
                *isActive1 = false;
                isConfigChanged = true;
            }
        }
    }
    else
    {
        jumpAddress = address2;

        if(version1 > version2)
        {
            if(!(*isChecked1))
            {
                jumpAddress = address1;
            }
            else if(*isGood1)
            {
                jumpAddress = address1;
                *isActive1 = true;
                *isActive2 = false;
                isConfigChanged = true;
            }
        }
    }

    if(isConfigChanged)
        writeConfig();

//  application = (void (*)(void))*jumpAddress;

    SysCtlPeripheralDisable(SYSCTL_PERIPH_EEPROM0);

    HWREG(NVIC_VTABLE) = jumpAddress; // relocate vector table

    boot_jump(jumpAddress);

//  application();
}


