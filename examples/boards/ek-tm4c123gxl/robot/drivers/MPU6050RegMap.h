#ifndef MPU6050_REG_MAP_H
#define MPU6050_REG_MAP_H


#define SELF_TEST_X 				0x0D
#define SELF_TEST_Y 				0x0E
#define SELF_TEST_Z 				0x0F
#define SELF_TEST_A 				0x10
#define SMPLRT_DIV 					0x19
#define CONFIG		 				0x1A
#define GYRO_CONFIG 				0x1B
#define ACCEL_CONFIG 				0x1C
#define MOT_THR		 				0x1F
#define FIFO_EN		 				0x23
#define I2C_MST_CTRL 				0x24
#define I2C_SLV0_ADDR 				0x25
#define I2C_SLV0_REG 				0x26
#define I2C_SLV0_CTRL 				0x27
#define I2C_SLV1_ADDR 				0x28
#define I2C_SLV1_REG 				0x29
#define I2C_SLV1_CTRL 				0x2A
#define I2C_SLV2_ADDR 				0x2B
#define I2C_SLV2_REG 				0x2C
#define I2C_SLV2_CTRL 				0x2D
#define I2C_SLV3_ADDR 				0x2E
#define I2C_SLV3_REG 				0x2F
#define I2C_SLV3_CTRL 				0x30
#define I2C_SLV4_ADDR 				0x31
#define I2C_SLV4_REG 				0x32
#define I2C_SLV4_DO 				0x33
#define I2C_SLV4_CTRL 				0x34
#define I2C_SLV4_DI 				0x35
#define I2C_MST_STATUS 				0x36
#define INT_PIN_CFG 				0x37
#define INT_ENABLE	 				0x38
#define INT_STATUS	 				0x3A
#define ACCEL_XOUT_H 				0x3B
#define ACCEL_XOUT_L 				0x3C
#define ACCEL_YOUT_H 				0x3D
#define ACCEL_YOUT_L 				0x3E
#define ACCEL_ZOUT_H 				0x3F
#define ACCEL_ZOUT_L 				0x40
#define TEMP_OUT_H	 				0x41
#define TEMP_OUT_L	 				0x42
#define GYRO_XOUT_H 				0x43
#define GYRO_XOUT_L 				0x44
#define GYRO_YOUT_H 				0x45
#define GYRO_YOUT_L 				0x46
#define GYRO_ZOUT_H 				0x47
#define GYRO_ZOUT_L 				0x48
// skip ext sensor data registers
#define I2C_SLV0_DO 				0x63
#define I2C_SLV1_DO 				0x64
#define I2C_SLV2_DO 				0x65
#define I2C_SLV3_DO 				0x66
#define I2C_MST_DELAY_CTRL			0x67
#define SIGNAL_PATH_RESET 			0x68
#define MOT_DETECT_CTRL 			0x69
#define USER_CTRL	 				0x6A
#define PWR_MGMT_1	 				0x6B
#define PWR_MGMT_2	 				0x6C
#define FIFO_COUNTH 				0x72
#define FIFO_COUNTL 				0x73
#define FIFO_R_W	 				0x74
#define WHO_AM_I	 				0x75



// register fields

// CONFIG
#define CONFIG__DLPF_CFG(x) (x & 0x7)

// GYRO_CONFIG
#define GYRO_CONFIG__XG_ST (1 << 7)
#define GYRO_CONFIG__YG_ST (1 << 6)
#define GYRO_CONFIG__YG_ST (1 << 5)
#define GYRO_CONFIG__FS_SEL(x) ((x & 0x3) << 3)

// ACCEL_CONFIG
#define ACCEL_CONFIG__XG_ST (1 << 7)
#define ACCEL_CONFIG__YG_ST (1 << 6)
#define ACCEL_CONFIG__YG_ST (1 << 5)
#define ACCEL_CONFIG__FS_SEL(x) ((x & 0x3) << 3)

// INT_PIN_CFG
#define INT_PIN_CFG__INT_LEVEL (1 << 7)
#define INT_PIN_CFG__INT_OPEN (1 << 6)
#define INT_PIN_CFG__LATCH_INT_EN (1 << 5)
#define INT_PIN_CFG__INT_RD_CLEAR1 (1 << 4)
#define INT_PIN_CFG__INT_FSYNC_INT_LEVEL (1 << 3)
#define INT_PIN_CFG__FSYNC_INT_EN (1 << 2)
#define INT_PIN_CFG__I2C_BYPASS_EN (1 << 1)

// INT_ENABLE
#define INT_ENABLE__MOT_EN (1 << 6)
#define INT_ENABLE__FIFO_OFLOW_EN (1 << 4)
#define INT_ENABLE__I2C_MST_INT_EN (1 << 3)
#define INT_ENABLE__DATA_RDY_EN (1 << 0)

// INT_STATUS
#define INT_STATUS__MOT_EN (1 << 6)
#define INT_STATUS__FIFO_OFLOW_EN (1 << 4)
#define INT_STATUS__I2C_MST_INT_EN (1 << 3)
#define INT_STATUS__DATA_RDY_EN (1 << 0)

// SIGNAL_PATH_RESET
#define SIGNAL_PATH_RESET__GYRO_RESET (1 << 2)
#define SIGNAL_PATH_RESET__ACCEL_RESET (1 << 1)
#define SIGNAL_PATH_RESET__TEMP_RESET (1 << 0)

// MOT_DETECT_CTRL
#define MOT_DETECT_CTRL__ACCEL_ON_DELAY(x) ((x & 0x3) << 4)

// USER_CTRL
#define USER_CTRL__FIFO_EN (1 << 6)
#define USER_CTRL__I2C_MST_EN (1 << 5)
#define USER_CTRL__I2C_IF_DIS (1 << 4)
#define USER_CTRL__FIFO_RESET (1 << 2)
#define USER_CTRL__I2C_MST_RESET (1 << 1)
#define USER_CTRL__SIG_COND_RESET (1 << 0)

// PWR_MGMT_1
#define PWR_MGMT_1__DEVICE_RESET (1 << 7)
#define PWR_MGMT_1__SLEEP (1 << 6)
#define PWR_MGMT_1__CYCLE (1 << 5)
#define PWR_MGMT_1__TEMP_DIS (1 << 3)
#define PWR_MGMT_1__CLKSEL(x) ((x & 0x7) << 0)

// PWR_MGMT_2
#define PWR_MGMT_2__LP_WAKE_CTRL(x) ((x & 0x3) << 6)
#define PWR_MGMT_2__STBY_XA (1 << 5)
#define PWR_MGMT_2__STBY_YA (1 << 4)
#define PWR_MGMT_2__CSTBY_ZA (1 << 3)
#define PWR_MGMT_2__STBY_XG (1 << 2)
#define PWR_MGMT_2__STBY_YG (1 << 1)
#define PWR_MGMT_2__STBY_ZG (1 << 0)

// WHO_AM_I
#define WHO_AM_I__WHO_AM_I(x) ((x & 0x3F) << 6)


#endif // MPU6050_REG_MAP_H
