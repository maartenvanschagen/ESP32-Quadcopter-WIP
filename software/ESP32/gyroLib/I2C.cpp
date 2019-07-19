#include "I2C.h"
#include "driver/i2c.h"
#include <cstdint>

//TODO: create overloads with i2cport for everything

namespace I2C{
  uint8_t getRegister(uint8_t device, uint8_t reg){
    uint8_t receiveData;
    getRegister(device, reg, &receiveData, 1);
    return receiveData;
  }

  bool getRegister(uint8_t device, uint8_t reg, uint8_t * receiveData, int amount){//TODO: create overloads for timeout
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();                                  //create command to tell device what to send and read the registers
    i2c_master_start(cmd);                                                         //startbit
    i2c_master_write_byte(cmd, (device << 1) | I2C_MASTER_WRITE, 0x1);             //send device address and tell it that the master is writing , ack check is enabled
    i2c_master_write_byte(cmd, reg, 0x1);                                          //tell it what register to send , enables the increment register address bit , ack check is enabled
    i2c_master_start(cmd);                                                         //repeated startbit to tell it to send stuf to the master
    i2c_master_write_byte(cmd, (device << 1) | I2C_MASTER_READ, 0x1);              //send device address and tell it that the master is reading and it needs to write , ack check is enabled
    if(amount > 1){
      i2c_master_read(cmd, receiveData, amount-1, I2C_MASTER_ACK);                 //give the device space to write
    }
    i2c_master_read(cmd, receiveData+amount-1, 1, I2C_MASTER_LAST_NACK);           //give the device space for its last bit and tell the device it is done writing
    i2c_master_stop(cmd);                                                          //stopbit
  
    i2c_master_cmd_begin(I2C_NUM_0, cmd, 10000);                                   //send first I2C message
    
    i2c_cmd_link_delete(cmd);                                                      //delete first message
    
    return true;                                                                    //TODO: make return usefull, return true if successful
  }

  void writeRegister(uint8_t device, uint8_t reg, uint8_t data){                    //TODO: return true if successful
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();                                   //create command message
    i2c_master_start(cmd);                                                          //startbit
    i2c_master_write_byte(cmd, (device << 1) | I2C_MASTER_WRITE, 0x1);              //send device address and tell it that the master is writing , ack check is enabled
    i2c_master_write_byte(cmd, reg, 0x1);                                           //tell device what register to write , ack check is enabled
    i2c_master_write_byte(cmd, data, 0x1);                                          //tell device what to write in register , ack check is enabled
    i2c_master_stop(cmd);                                                           //stopbit
    i2c_master_cmd_begin(I2C_NUM_0, cmd, 10000);                                    //send I2C message
    i2c_cmd_link_delete(cmd);                                                       //delete message
  }

  void writeRegister(uint8_t device, uint8_t reg, uint8_t dataBits, uint8_t data){
    uint8_t writeData = getRegister(device, reg);                               //first get the register
    uint8_t all1 = dataBits & data;                                                 //find all the spots to write a 1 to in the register
    uint8_t all0 = dataBits & ~data;                                                //find all the spots to write a 0 to in the register 
    writeData |= all1;                                                              //write all the 1's
    writeData &= ~all0;                                                             //write all the 0's
    
    writeRegister(device, reg, writeData);                                      //write it to the device register
  }

  void init(){ //TODO: make config changable with arguments (pins as arguments or i2c_config as argument)
    i2c_config_t conf;
    conf.mode = I2C_MODE_MASTER;
    conf.sda_io_num = GPIO_NUM_16;
    conf.sda_pullup_en = GPIO_PULLUP_ENABLE;
    conf.scl_io_num = GPIO_NUM_17;
    conf.scl_pullup_en = GPIO_PULLUP_ENABLE;
    conf.master.clk_speed = 400000;
    i2c_param_config(I2C_NUM_0, &conf);
    i2c_driver_install(I2C_NUM_0, conf.mode, 0, 0, 0); //setup i2c with device 0, set mode to master with no receive and send buffer , no flags
  }
}
