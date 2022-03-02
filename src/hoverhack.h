#include <stdint.h>
#include <cstddef>
#include <Arduino.h>

namespace hoverhack
{
  typedef struct
  {
    int16_t steer;
    int16_t speed;
    uint32_t crc;
  } Serialcommand;
  Serialcommand oCmd;

  typedef struct
  {
    int16_t iSpeedL; // 100* km/h
    int16_t iSpeedR; // 100* km/h
    uint16_t iHallSkippedL;
    uint16_t iHallSkippedR;
    uint16_t iTemp; // °C
    uint16_t iVolt; // 100* V
    int16_t iAmpL;  // 100* A
    int16_t iAmpR;  // 100* A
    uint32_t crc;
  } SerialFeedback;
  SerialFeedback oFeedback;

  uint32_t crc32_for_byte(uint32_t r)
  {
    for (int j = 0; j < 8; ++j)
    {
      r = (r & 1 ? 0 : (uint32_t)0xEDB88320L) ^ r >> 1;
    }
    return r ^ (uint32_t)0xFF000000L;
  }

  void crc32(const void *data, size_t n_bytes, uint32_t *crc)
  {
    static uint32_t table[0x100];
    if (!*table)
    {
      for (size_t i = 0; i < 0x100; ++i)
      {
        table[i] = crc32_for_byte(i);
      }
    }
    for (size_t i = 0; i < n_bytes; ++i)
    {
      *crc = table[(uint8_t)*crc ^ ((uint8_t *)data)[i]] ^ *crc >> 8;
    }
  }

  void hoverSend(int16_t iSpeed, int16_t iSteer)
  {
    oCmd.steer = iSteer;
    oCmd.speed = iSpeed;

    uint32_t crc = 0;
    crc32((const void *)&oCmd, sizeof(Serialcommand) - 4, &crc);
    oCmd.crc = crc;

    Serial.write((uint8_t *)&oCmd, sizeof(oCmd));
  }

  int iFailedRec = 0;

  boolean hoverReceive()
  {
    if (Serial.available() < sizeof(SerialFeedback))
    {
      return false;
    }

    SerialFeedback oNew;
    byte *p = (byte *)&oNew;
    for (unsigned int i = 0; i < sizeof(SerialFeedback); i++)
    {
      *p++ = Serial.read();
    }

    uint32_t crc = 0;
    crc32((const void *)&oNew, sizeof(SerialFeedback) - 4, &crc);

    if (oNew.crc == crc)
    {
      memcpy(&oFeedback, &oNew, sizeof(SerialFeedback));
      return true;
    }

    return false;
  }
}
