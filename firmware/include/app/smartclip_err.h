#ifndef APP_SMARTCLIP_ERR_H
#define APP_SMARTCLIP_ERR_H

typedef enum {
    SMARTCLIP_OK = 0,
    SMARTCLIP_ERR_INVALID_ARG    = -1,
    SMARTCLIP_ERR_NOT_INITIALIZED= -2,
    SMARTCLIP_ERR_BUSY           = -3,
    SMARTCLIP_ERR_TIMEOUT        = -4,
    SMARTCLIP_ERR_BUS            = -5,
    SMARTCLIP_ERR_NACK           = -6,
    SMARTCLIP_ERR_BAD_ID         = -7,
    SMARTCLIP_ERR_CRC            = -8,
    SMARTCLIP_ERR_NO_MEM         = -9,
    SMARTCLIP_ERR_NOT_SUPPORTED  =-10,
    SMARTCLIP_ERR_INTERNAL       =-99,
} smartclip_err_t;

const char * smartclip_err_str(smartclip_err_t e);

#endif