#ifndef __LSM303_H__
#define __LSM303_H__

#include <inttypes.h>
#include <stdbool.h>

#define LSM303A_I2C_ADDR  0x32
#define LSM303M_I2C_ADDR  0x3C

#define LSM303M_TIMER_STEP_MS  200

typedef struct {
  float x;
  float y;
  float z;
} LSM303_VALUES;

typedef struct {
  float roll;
  float pitch;
  float yaw;
} LSM303_ANGLES;

#define LSM303M_I2C_BURST_ADDR_BIT          (1 << 7)

#define LSM303A_CTRL_REG1                   0x20
#define LSM303A_CTRL_REG1_ENABLE_X          (1 << 0)
#define LSM303A_CTRL_REG1_ENABLE_Y          (1 << 1)
#define LSM303A_CTRL_REG1_ENABLE_Z          (1 << 2)
#define LSM303A_CTRL_REG1_LOW_POWER_MODE    (1 << 3)
#define LSM303A_CTRL_REG1_POWER_DOWN        (0 << 4)
#define LSM303A_CTRL_REG1_DATARATE_1_HZ     (1 << 4)
#define LSM303A_CTRL_REG1_DATARATE_10_HZ    (2 << 4)
#define LSM303A_CTRL_REG1_DATARATE_25_HZ    (3 << 4)
#define LSM303A_CTRL_REG1_DATARATE_50_HZ    (4 << 4)
#define LSM303A_CTRL_REG1_DATARATE_100_HZ   (5 << 4)
#define LSM303A_CTRL_REG1_DATARATE_200_HZ   (6 << 4)
#define LSM303A_CTRL_REG1_DATARATE_400_HZ   (7 << 4)
#define LSM303A_CTRL_REG1_DATARATE_1620_HZ  (8 << 4)
#define LSM303A_CTRL_REG1_DATARATE_1344_HZ  (9 << 4)
#define LSM303A_CTRL_REG1_DATARATE_MASK     0xF0

#define LSM303A_CTRL_REG2                   0x21
#define LSM303A_CTRL_REG2_HPF_EN_IS1        (1 << 0)
#define LSM303A_CTRL_REG2_HPF_EN_IS2        (1 << 1)
#define LSM303A_CTRL_REG2_HPF_EN_CLICK      (1 << 2)
#define LSM303A_CTRL_REG2_FDS               (1 << 3)
#define LSM303A_CTRL_REG2_HPF_MOD_NRES      (0 << 6)
#define LSM303A_CTRL_REG2_HPF_MOD_REF       (1 << 6)
#define LSM303A_CTRL_REG2_HPF_MOD_NORM      (2 << 6)
#define LSM303A_CTRL_REG2_HPF_MOD_AUTO      (3 << 6)
#define LSM303A_CTRL_REG2_HPF_MOD_MASK      0xC0

#define LSM303A_CTRL_REG3                   0x22
#define LSM303A_CTRL_REG3_I1_OVERRUN        (1 << 1)
#define LSM303A_CTRL_REG3_I1_FIFO_WTR       (1 << 2)
#define LSM303A_CTRL_REG3_I1_DRDY2          (1 << 3)
#define LSM303A_CTRL_REG3_I1_DRDY1          (1 << 4)
#define LSM303A_CTRL_REG3_I1_AOI2           (1 << 5)
#define LSM303A_CTRL_REG3_I1_AOI1           (1 << 6)
#define LSM303A_CTRL_REG3_I1_CLICK          (1 << 7)

#define LSM303A_CTRL_REG4                   0x23
#define LSM303A_CTRL_REG4_SPI_MODE          (1 << 0)
#define LSM303A_CTRL_REG4_HIRES_OUT         (1 << 3)
#define LSM303A_CTRL_REG4_FULLSCALE_2G      (0 << 4)
#define LSM303A_CTRL_REG4_FULLSCALE_4G      (1 << 4)
#define LSM303A_CTRL_REG4_FULLSCALE_8G      (2 << 4)
#define LSM303A_CTRL_REG4_FULLSCALE_16G     (3 << 4)
#define LSM303A_CTRL_REG4_FULLSCALE_MASK    0x30
#define LSM303A_CTRL_REG4_BIG_ENDIAN        (1 << 6)
#define LSM303A_CTRL_REG4_BLOCK_UPDATE      (1 << 7)

#define LSM303A_SENSITIVITY_FIFO            0.25
#define LSM303A_SENSITIVITY_2G              1.0
#define LSM303A_SENSITIVITY_4G              2.0
#define LSM303A_SENSITIVITY_8G              4.0
#define LSM303A_SENSITIVITY_16G             12.0

#define LSM303A_CTRL_REG5                   0x24
#define LSM303A_CTRL_REG5_D4D_INT2          (1 << 0)
#define LSM303A_CTRL_REG5_LIR_INT2          (1 << 1)
#define LSM303A_CTRL_REG5_D4D_INT1          (1 << 2)
#define LSM303A_CTRL_REG5_LIR_INT1          (1 << 3)
#define LSM303A_CTRL_REG5_FIFO_ENABLE       (1 << 6)
#define LSM303A_CTRL_REG5_REBOOT_MEMORY     (1 << 7)

#define LSM303A_CTRL_REG6                   0x25
#define LSM303A_CTRL_REG6_INT_H_ACTIVE      (1 << 1)
#define LSM303A_CTRL_REG6_PAD2_ACT          (1 << 3)
#define LSM303A_CTRL_REG6_PAD2_BOOT         (1 << 4)
#define LSM303A_CTRL_REG6_PAD2_INT2         (1 << 5)
#define LSM303A_CTRL_REG6_PAD2_INT1         (1 << 6)
#define LSM303A_CTRL_REG6_PAD2_CLICK        (1 << 7)

#define LSM303A_REFERENCE                   0x26

#define LSM303A_STATUS_REG                  0x27
#define LSM303A_STATUS_REG_READY_X          (1 << 0)
#define LSM303A_STATUS_REG_READY_Y          (1 << 1)
#define LSM303A_STATUS_REG_READY_Z          (1 << 2)
#define LSM303A_STATUS_REG_READY_XYZ        (1 << 3)
#define LSM303A_STATUS_REG_OVERRUN_X        (1 << 4)
#define LSM303A_STATUS_REG_OVERRUN_Y        (1 << 5)
#define LSM303A_STATUS_REG_OVERRUN_Z        (1 << 6)
#define LSM303A_STATUS_REG_OVERRUN_XYZ      (1 << 7)

#define LSM303A_OUT_X_L                     0x28
#define LSM303A_OUT_X_H                     0x29
#define LSM303A_OUT_Y_L                     0x2A
#define LSM303A_OUT_Y_H                     0x2B
#define LSM303A_OUT_Z_L                     0x2C
#define LSM303A_OUT_Z_H                     0x2D

#define LSM303A_FIFO_CTRL_REG               0x2E
#define LSM303A_FIFO_CTRL_REG_TRIGGER_INT1  (0 << 5)
#define LSM303A_FIFO_CTRL_REG_TRIGGER_INT2  (1 << 5)
#define LSM303A_FIFO_CTRL_REG_MODE_BYPASS   (0 << 6)
#define LSM303A_FIFO_CTRL_REG_MODE_FIFO     (1 << 6)
#define LSM303A_FIFO_CTRL_REG_MODE_STREAM   (2 << 6)
#define LSM303A_FIFO_CTRL_REG_MODE_TRIGGER  (3 << 6)
#define LSM303A_FIFO_CTRL_REG_MODE_MASK     0xC0

#define LSM303A_FIFO_SRC_REG                0x2F
#define LSM303A_FIFO_SRC_REG_EMPTY          (1 << 5)
#define LSM303A_FIFO_SRC_REG_OVERRUN        (1 << 6)
#define LSM303A_FIFO_SRC_REG_WATERMARK      (1 << 7)

#define LSM303A_INT1_CFG                    0x30
#define LSM303A_INT_CFG_X_LOW_EVENT         (1 << 0)
#define LSM303A_INT_CFG_X_HIGH_EVENT        (1 << 1)
#define LSM303A_INT_CFG_Y_LOW_EVENT         (1 << 2)
#define LSM303A_INT_CFG_Y_HIGH_EVENT        (1 << 3)
#define LSM303A_INT_CFG_Z_LOW_EVENT         (1 << 4)
#define LSM303A_INT_CFG_Z_HIGH_EVENT        (1 << 5)
#define LSM303A_INT_CFG_6D                  (1 << 6)
#define LSM303A_INT_CFG_AOI                 (1 << 7)

#define LSM303A_INT1_SOURCE                 0x31
#define LSM303A_INT_X_LOW                   (1 << 0)
#define LSM303A_INT_X_HIGH                  (1 << 1)
#define LSM303A_INT_Y_LOW                   (1 << 2)
#define LSM303A_INT_Y_HIGH                  (1 << 3)
#define LSM303A_INT_Z_LOW                   (1 << 4)
#define LSM303A_INT_Z_HIGH                  (1 << 5)
#define LSM303A_INT_INTERRUPT_ACTIVE        (1 << 6)

#define LSM303A_INT1_THRESHOLD              0x32
#define LSM303A_INT1_DURATION               0x33

#define LSM303A_INT2_CFG                    0x34
#define LSM303A_INT2_SOURCE                 0x35
#define LSM303A_INT2_THRESHOLD              0x36
#define LSM303A_INT2_DURATION               0x37

#define LSM303A_CLICK_CFG                   0x38
#define LSM303A_CLICK_CFG_X_SINGLE_ENABLE   (1 << 0)
#define LSM303A_CLICK_CFG_X_DOUBLE_ENABLE   (1 << 1)
#define LSM303A_CLICK_CFG_Y_SINGLE_ENABLE   (1 << 2)
#define LSM303A_CLICK_CFG_Y_DOUBLE_ENABLE   (1 << 3)
#define LSM303A_CLICK_CFG_Z_SINGLE_ENABLE   (1 << 4)
#define LSM303A_CLICK_CFG_Z_DOUBLE_ENABLE   (1 << 5)

#define LSM303A_CLICK_SOURCE                0x39
#define LSM303A_CLICK_SOURCE_X              (1 << 0)
#define LSM303A_CLICK_SOURCE_Y              (1 << 1)
#define LSM303A_CLICK_SOURCE_Z              (1 << 2)
#define LSM303A_CLICK_SOURCE_SIGN           (1 << 3)
#define LSM303A_CLICK_SOURCE_SINGLE         (1 << 4)
#define LSM303A_CLICK_SOURCE_DOUBLE         (1 << 5)
#define LSM303A_CLICK_SOURCE_INT_ACTIVE     (1 << 6)

#define LSM303A_CLICK_THRESHOLD             0x3A

#define LSM303A_TIME_LIMIT                  0x3B
#define LSM303A_TIME_LATENCY                0x3C
#define LSM303A_TIME_WINDOW                 0x3D

#define LSM303M_CRA_REG                     0x00
#define LSM303M_CRA_REG_DATARATE_0_75_HZ    (0 << 3)
#define LSM303M_CRA_REG_DATARATE_1_5_HZ     (1 << 3)
#define LSM303M_CRA_REG_DATARATE_3_HZ       (2 << 3)
#define LSM303M_CRA_REG_DATARATE_7_5_HZ     (3 << 3)
#define LSM303M_CRA_REG_DATARATE_15_HZ      (4 << 3)
#define LSM303M_CRA_REG_DATARATE_30_HZ      (5 << 3)
#define LSM303M_CRA_REG_DATARATE_75_HZ      (6 << 3)
#define LSM303M_CRA_REG_DATARATE_220_HZ     (7 << 3)
#define LSM303M_CRA_REG_DATARATE_MASK       0x1C
#define LSM303M_CRA_REG_TEMP_SENSOR_ENABLE  (1 << 7)

#define LSM303M_CRB_REG                     0x01
#define LSM303M_CRB_REG_FULLSCALE_1_3GA     (1 << 5)
#define LSM303M_CRB_REG_FULLSCALE_1_9GA     (2 << 5)
#define LSM303M_CRB_REG_FULLSCALE_2_5GA     (3 << 5)
#define LSM303M_CRB_REG_FULLSCALE_4_GA      (4 << 5)
#define LSM303M_CRB_REG_FULLSCALE_4_7_GA    (5 << 5)
#define LSM303M_CRB_REG_FULLSCALE_5_6_GA    (6 << 5)
#define LSM303M_CRB_REG_FULLSCALE_8_1_GA    (7 << 5)
#define LSM303M_CRB_REG_FULLSCALE_MASK      0xE0

#define LSM303M_SENSITIVITY_XY_1_3GA        (1 / 1100.0)
#define LSM303M_SENSITIVITY_XY_1_9GA        (1 / 855.0)
#define LSM303M_SENSITIVITY_XY_2_5GA        (1 / 670.0)
#define LSM303M_SENSITIVITY_XY_4GA          (1 / 450.0)
#define LSM303M_SENSITIVITY_XY_4_7GA        (1 / 400.0)
#define LSM303M_SENSITIVITY_XY_5_6GA        (1 / 330.0)
#define LSM303M_SENSITIVITY_XY_8_1GA        (1 / 230.0)
#define LSM303M_SENSITIVITY_Z_1_3GA         (1 / 980.0)
#define LSM303M_SENSITIVITY_Z_1_9GA         (1 / 760.0)
#define LSM303M_SENSITIVITY_Z_2_5GA         (1 / 600.0)
#define LSM303M_SENSITIVITY_Z_4GA           (1 / 400.0)
#define LSM303M_SENSITIVITY_Z_4_7GA         (1 / 355.0)
#define LSM303M_SENSITIVITY_Z_5_6GA         (1 / 295.0)
#define LSM303M_SENSITIVITY_Z_8_1GA         (1 / 205.0)

#define LSM303M_MR_REG                      0x02
#define LSM303M_MR_REG_MODE_CONTINIOUS      (0 << 0)
#define LSM303M_MR_REG_MODE_SINGLE          (1 << 0)
#define LSM303M_MR_REG_MODE_SLEEP           (2 << 0)
#define LSM303M_MR_REG_MODE_MASK            0x03

#define LSM303M_OUT_X_H                     0x03
#define LSM303M_OUT_X_L                     0x04
#define LSM303M_OUT_Z_H                     0x05
#define LSM303M_OUT_Z_L                     0x06
#define LSM303M_OUT_Y_H                     0x07
#define LSM303M_OUT_Y_L                     0x08

#define LSM303M_SR_REG                      0x09
#define LSM303M_SR_REG_READY                (1 << 0)
#define LSM303M_SR_REG_LOCK                 (1 << 1)

#define LSM303M_IRA_REG                     0x0A
#define LSM303M_IRB_REG                     0x0B
#define LSM303M_IRC_REG                     0x0C

#define LSM303M_IRA                         0x48
#define LSM303M_IRB                         0x34
#define LSM303M_IRC                         0x33

#define LSM303M_OUT_TEMP_H                  0x31
#define LSM303M_OUT_TEMP_L                  0x32

LSM303_VALUES lsm303_accelReal;
LSM303_VALUES lsm303_magnetReal;
LSM303_ANGLES lsm303_anglesReal;

bool lsm303_used;
bool lsm303_error;

bool lsm303a_init(void);
bool lsm303m_init(void);

bool lsm303a_read(LSM303_VALUES* values);
bool lsm303m_read(LSM303_VALUES* values);

bool lsm303_init(void);
void lsm303_start(void);
bool lsm303_get(LSM303_ANGLES* angles);

#endif // __LSM303_H__
