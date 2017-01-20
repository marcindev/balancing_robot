#include "gtest/gtest.h"
#include <stdlib.h>
#include "gmock/gmock.h"
extern "C" {
#include "memoryDevice.c"
}
#include "testFixture.h"
#include "helper.h"
#include <vector>
#include <algorithm>
#include <cstring>
#include <string>
#include <iterator>

using ::testing::_;
using ::testing::Return;
using ::testing::InSequence;
using ::testing::Invoke;

extern "C" {

void ZeroBuffer(void* buffer, int len)
{
    unsigned char* uchPtr = (unsigned char*) buffer;

    for(int i = 0; i != len; i++)
    {
        *uchPtr++ = 0;
    }
}

}

namespace RobotTest {


class MemoryDeviceTest : public TestFixture, public Helper 
{
public:
    MemoryDeviceTest() :
	_memory(new uint8_t[MEM_SIZE])
    {
	_memSize = MEM_SIZE;
	_memBeg = reinterpret_cast<uint32_t>(_memory);
	_memEnd = reinterpret_cast<uint32_t>(_memory) + _memSize;
	_smallBlockSize = 4;
	_largeBlockSize = 10;
	_eraseBlockSize = 10;
	_isReadFail = false;
	_isWriteFail = false;

	initMemory();

	ON_CALL(*_rtosMock, pvPortMalloc(_))
	    .WillByDefault(Invoke(this, &MemoryDeviceTest::mallocWrapper));
	ON_CALL(*_rtosMock, vPortFree(_))
	    .WillByDefault(Invoke(this, &MemoryDeviceTest::freeWrapper));

	_device = MemDevCreate();
	registerCallbacks();
    }

    ~MemoryDeviceTest()
    {
	delete[] _memory;
    }


    
    void registerCallbacks()
    {
	MemDevRegisterInitFunc(_device, initFuncMdTest);
	MemDevRegisterGetParamsFunc(_device, getParamsFuncMdTest);
	MemDevRegisterReadFunc(_device, readFuncMdTest);
	MemDevRegisterWriteFunc(_device, writeFuncMdTest);
	MemDevRegisterEraseFunc(_device, eraseFuncMdTest);
    }

    void initMemory()
    {
	std::memset(_memory, 0, _memEnd - _memBeg);	
    }

    static int32_t initFuncMdTest()
    {
	return 0; 
    }

    static int32_t readFuncMdTest(MemAddress addr, MemData* data, MemSize size)
    {
	if(_isReadFail)
	    return -1;

	uint8_t* memPtr = reinterpret_cast<uint8_t*>(addr);

	if(addr < _memBeg || (addr + size)  >= _memEnd)
	    return -1;

	std::copy(memPtr, memPtr + size, static_cast<uint8_t*>(data));

	return 0;
    }

    static int32_t writeFuncMdTest(MemAddress addr, MemData* data, MemSize size)
    {
	if(_isWriteFail)
	    return -1;

	uint8_t* memPtr = reinterpret_cast<uint8_t*>(addr);

	if(addr < _memBeg || (addr + size)  >= _memEnd)
	    return -1;

	std::copy(static_cast<uint8_t*>(data), static_cast<uint8_t*>(data) + size, memPtr);

	return 0;
    }

    static int32_t eraseFuncMdTest(MemAddress, MemSize)
    {
	uint8_t* memPtr = reinterpret_cast<uint8_t*>(addr);

	if(addr < _memBeg || (addr + size)  >= _memEnd)
	    return -1;

	std::copy(static_cast<uint8_t*>(data), static_cast<uint8_t*>(data) + size, memPtr);

	return 0;
    }

    static int32_t getParamsFuncMdTest(MemDevParams* params)
    {
	params->begAddr = _memBeg;
	params->endAddr = _memEnd;
	params->smallBlockSize = _smallBlockSize;
	params->largeBlockSize = _largeBlockSize;
	params->eraseBlockSize = _eraseBlockSize;

	return 0;

    }

    void testMemDevRead(const std::string& expectedStr, uint32_t readSize)
    {
	char out[readSize];
	std::memset(out, 0, readSize);
	std::copy(expectedStr.cbegin(), expectedStr.cend(), _memory);

	ASSERT_TRUE(MemDevOpen(_device));
	ASSERT_TRUE(MemDevRead(_device, _memBeg, out, readSize));
	EXPECT_EQ(expectedStr, std::string(out));
	EXPECT_TRUE(MemDevClose(_device));

	std::memset(out, 0, readSize);
	std::copy(expectedStr.cbegin(), expectedStr.cend(), _memory + _memSize / 2);

	ASSERT_TRUE(MemDevOpen(_device));
	ASSERT_TRUE(MemDevRead(_device, _memBeg + _memSize / 2, out, readSize));
	EXPECT_EQ(expectedStr, std::string(out));
	EXPECT_TRUE(MemDevClose(_device));
    }

    void testMemDevWrite(const std::string& strToWrite)
    {
	const uint32_t writeSize = strToWrite.size();
	char in[writeSize];
	std::memset(in, 0, writeSize);
	std::copy(strToWrite.cbegin(), strToWrite.cend(), in);
	std::string resultStr;

	ASSERT_TRUE(MemDevOpen(_device));
	ASSERT_TRUE(MemDevWrite(_device, _memBeg, in, writeSize));
	std::copy(_memory, _memory + writeSize, std::back_inserter(resultStr));
	EXPECT_EQ(strToWrite, resultStr);
	EXPECT_TRUE(MemDevClose(_device));
	resultStr.clear();

	ASSERT_TRUE(MemDevOpen(_device));
	ASSERT_TRUE(MemDevWrite(_device, _memBeg + _memSize / 2, in, writeSize));
	std::copy(_memory + _memSize / 2, _memory + _memSize / 2 + writeSize, std::back_inserter(resultStr));
	EXPECT_EQ(strToWrite, resultStr);
	EXPECT_TRUE(MemDevClose(_device));
	resultStr.clear();
    }


    static uint32_t _memSize,
		    _memBeg, 
		    _memEnd,
		    _smallBlockSize,
		    _largeBlockSize,
             	    _eraseBlockSize;

    static bool _isReadFail,
		_isWriteFail;

    static const uint32_t MEM_SIZE = 100;

protected:
    MemoryDevice _device;
    uint8_t* _memory;
};

uint32_t MemoryDeviceTest::_memSize;
uint32_t MemoryDeviceTest::_memBeg; 
uint32_t MemoryDeviceTest::_memEnd;
uint32_t MemoryDeviceTest::_smallBlockSize;
uint32_t MemoryDeviceTest::_largeBlockSize;
uint32_t MemoryDeviceTest::_eraseBlockSize;
bool MemoryDeviceTest::_isReadFail;
bool MemoryDeviceTest::_isWriteFail;


TEST_F(MemoryDeviceTest, MemDevCreate_create_success)
{
    EXPECT_CALL(*_rtosMock, pvPortMalloc(_)).WillOnce(Invoke(this, &MemoryDeviceTest::mallocWrapper));

    MemoryDevice device = MemDevCreate();

    EXPECT_NE(0, reinterpret_cast<int>(device));    
}

TEST_F(MemoryDeviceTest, MemDevOpen_deviceAlreadyOpened_fails)
{
    EXPECT_TRUE(MemDevOpen(_device));    
    EXPECT_FALSE(MemDevOpen(_device));
}

TEST_F(MemoryDeviceTest, MemDevRead_deviceIsNotOpened_fails)
{
    const uint32_t OUT_SIZE = 10;
    char out[OUT_SIZE];

    EXPECT_FALSE(MemDevRead(_device, _memBeg, out, OUT_SIZE));
}

TEST_F(MemoryDeviceTest, MemDevRead_addressOutOfRange_fails)
{
    const uint32_t OUT_SIZE = 10;
    char out[OUT_SIZE];

    ASSERT_TRUE(MemDevOpen(_device));
    EXPECT_FALSE(MemDevRead(_device, _memBeg - 1, out, OUT_SIZE));
    EXPECT_FALSE(MemDevRead(_device, _memEnd + 1, out, OUT_SIZE));
    EXPECT_FALSE(MemDevRead(_device, _memEnd, out, OUT_SIZE));
    EXPECT_TRUE(MemDevClose(_device));
}

TEST_F(MemoryDeviceTest, MemDevRead_dataSizeOutOfRange_fails)
{
    const uint32_t OUT_SIZE = _memSize + 1;
    char out[OUT_SIZE];

    ASSERT_TRUE(MemDevOpen(_device));
    EXPECT_FALSE(MemDevRead(_device, _memBeg, out, OUT_SIZE));
    EXPECT_FALSE(MemDevRead(_device, _memBeg + (_memSize/2), out, OUT_SIZE));
    EXPECT_FALSE(MemDevRead(_device, _memEnd - 1, out, OUT_SIZE));
    EXPECT_TRUE(MemDevClose(_device));
}

TEST_F(MemoryDeviceTest, MemDevRead_readSizeGreaterThanLargeBlock_returnsCorrectString)
{
    const uint32_t OUT_SIZE = 15;
    std::string expectedStr("Ala Ma Kota");

    testMemDevRead(expectedStr, OUT_SIZE);
}

TEST_F(MemoryDeviceTest, MemDevRead_readSizeLessThanLargeBlockAndGreaterThanSmall_returnsCorrectString)
{
    const uint32_t OUT_SIZE = 7;
    std::string expectedStr("Tamago");

    testMemDevRead(expectedStr, OUT_SIZE);
}

TEST_F(MemoryDeviceTest, MemDevRead_readSizeLessThanSmallBlock_returnsCorrectString)
{
    const uint32_t OUT_SIZE = 3;
    std::string expectedStr("As");

    testMemDevRead(expectedStr, OUT_SIZE);
}

TEST_F(MemoryDeviceTest, MemDevRead_readSizeEqualSmallBlock_returnsCorrectString)
{
    const uint32_t OUT_SIZE = _smallBlockSize;
    std::string expectedStr("Mod");

    testMemDevRead(expectedStr, OUT_SIZE);
}

TEST_F(MemoryDeviceTest, MemDevRead_readSizeEqualLargeBlock_returnsCorrectString)
{
    const uint32_t OUT_SIZE = _largeBlockSize;
    std::string expectedStr("AbCdE1234");

    testMemDevRead(expectedStr, OUT_SIZE);
}

TEST_F(MemoryDeviceTest, MemDevRead_readSizeIsMultipleOfLargeBlock_returnsCorrectString)
{
    const uint32_t OUT_SIZE = _largeBlockSize * 3 + 3;
    std::string expectedStr("->So many books, so little time.");

    testMemDevRead(expectedStr, OUT_SIZE);
}

/*------------------------------------*/

TEST_F(MemoryDeviceTest, MemDevWrite_deviceIsNotOpened_fails)
{
    const uint32_t IN_SIZE = 10;
    char in[IN_SIZE];

    EXPECT_FALSE(MemDevWrite(_device, _memBeg, in, IN_SIZE));
}

TEST_F(MemoryDeviceTest, MemDevWrite_addressOutOfRange_fails)
{
    const uint32_t IN_SIZE = 10;
    char in[IN_SIZE];

    ASSERT_TRUE(MemDevOpen(_device));
    EXPECT_FALSE(MemDevWrite(_device, _memBeg - 1, in, IN_SIZE));
    EXPECT_FALSE(MemDevWrite(_device, _memEnd + 1, in, IN_SIZE));
    EXPECT_FALSE(MemDevWrite(_device, _memEnd, in, IN_SIZE));
    EXPECT_TRUE(MemDevClose(_device));
}

TEST_F(MemoryDeviceTest, MemDevWrite_dataSizeOutOfRange_fails)
{
    const uint32_t IN_SIZE = _memSize + 1;
    char in[IN_SIZE];

    ASSERT_TRUE(MemDevOpen(_device));
    EXPECT_FALSE(MemDevWrite(_device, _memBeg, in, IN_SIZE));
    EXPECT_FALSE(MemDevWrite(_device, _memBeg + (_memSize/2), in, IN_SIZE));
    EXPECT_FALSE(MemDevWrite(_device, _memEnd - 1, in, IN_SIZE));
    EXPECT_TRUE(MemDevClose(_device));
}

TEST_F(MemoryDeviceTest, MemDevWrite_writeSizeGreaterThanLargeBlock_returnsCorrectString)
{
    std::string strToWrite("Ala Ma Kota");

    testMemDevWrite(strToWrite);
}

TEST_F(MemoryDeviceTest, MemDevWrite_writeSizeLessThanLargeBlockAndGreaterThanSmall_returnsCorrectString)
{
    std::string strToWrite("Tamago");

    testMemDevWrite(strToWrite);
}

TEST_F(MemoryDeviceTest, MemDevWrite_writeSizeLessThanSmallBlock_returnsCorrectString)
{
    std::string strToWrite("As");

    testMemDevWrite(strToWrite);
}

TEST_F(MemoryDeviceTest, MemDevWrite_writeSizeEqualSmallBlock_returnsCorrectString)
{
    std::string strToWrite("Moda");

    testMemDevWrite(strToWrite);
}

TEST_F(MemoryDeviceTest, MemDevWrite_writeSizeEqualLargeBlock_returnsCorrectString)
{
    std::string strToWrite("AbCdE12345");

    testMemDevWrite(strToWrite);
}

TEST_F(MemoryDeviceTest, MemDevWrite_writeSizeIsMultipleOfLargeBlock_returnsCorrectString)
{
    std::string strToWrite("->So many books, so little time.");

    testMemDevWrite(strToWrite);
}


}
