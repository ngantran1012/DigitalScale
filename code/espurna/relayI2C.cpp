#include "relayI2C.h"
#include "config/hardware.h"
#if RELAY_CARD_SUPPORT

#include <vector>
#include "Wire.h"
#include "i2c.h"
#include "relay.h"

#define RELAY_CARD_ONE_OUTPUT         1
#define TRIAC_CARD_ONE_OUTPUT         2
#define RELAY_CARD_TWO_OUTPUT         3
#define RELAY_CARD_THREE_OUTPUT       4
#define RELAY_CARD_FOUR_OUTPUT        5

#define REG_OUTPUT_DATA         0

typedef struct {
    unsigned char address;
    unsigned char type;
} relayCard_t;

typedef struct {
    relayCard_t card;
    uint8_t output_id[4];
    uint8_t nOutput;
    uint8_t cardData;
} card_output_t;

std::vector<card_output_t> _out_cards;
// extern std::vector<relay_t> _relays;

char _relayError = 0;

void RelayCardSetup(void) {
    u8 error;
    for (u8 address = 0x10; address < 0x16; address++) {

        error = i2cCheck(address);

        if (error == 0) {
            uint8_t type = i2c_read_uint8(address);
            
            //TODO: change type to QCARD_ONE_OUTPUT, QCARD_TWO_OUTPUTS,...
            switch(type) {
                case RELAY_CARD_ONE_OUTPUT:
                case RELAY_CARD_TWO_OUTPUT:
                case RELAY_CARD_THREE_OUTPUT:
                case RELAY_CARD_FOUR_OUTPUT:
                    card_output_t output;
                    output.card.address = address;
                    output.card.type = type;
                    _out_cards.push_back(output);
                    DEBUG_MSG_P(PSTR("[CARD] Output card found at address 0x%02X\n"), address);
                    break;
            }
        }
    }
}

void OutputRelaySetup(void) {
    uint8_t iCard;

    for(iCard = 0; iCard < _out_cards.size(); iCard++) {
        switch (_out_cards[iCard].card.type) {
            case RELAY_CARD_ONE_OUTPUT:
                _out_cards[iCard].nOutput = 1;
                _out_cards[iCard].cardData = 0;

                _out_cards[iCard].output_id[0] = _relayAdd();
                break;
            case RELAY_CARD_TWO_OUTPUT:
                _out_cards[iCard].nOutput = 2;
                _out_cards[iCard].cardData = 0;

                _out_cards[iCard].output_id[0] = _relayAdd();
                _out_cards[iCard].output_id[1] = _relayAdd();
                break;
            case RELAY_CARD_THREE_OUTPUT:
                _out_cards[iCard].nOutput = 3;
                _out_cards[iCard].cardData = 0;

                _out_cards[iCard].output_id[0] = _relayAdd();
                _out_cards[iCard].output_id[1] = _relayAdd();
                _out_cards[iCard].output_id[2] = _relayAdd();
                break;
            case RELAY_CARD_FOUR_OUTPUT:
                _out_cards[iCard].nOutput = 4;
                _out_cards[iCard].cardData = 0;

                _out_cards[iCard].output_id[0] = _relayAdd();
                _out_cards[iCard].output_id[1] = _relayAdd();
                _out_cards[iCard].output_id[2] = _relayAdd();
                _out_cards[iCard].output_id[3] = _relayAdd();
                break;
            default:
                DEBUG_MSG_P(PSTR("[CARD] Output card type (%d) doesn't exist.\n"), _out_cards[iCard].card.type);
                break;
        }
    }
}

char getI2CRelayError() {
    return _relayError;
}

void RelayCardStatus(unsigned char id, bool status) {
    uint8_t iCard, iOut, iTry;

    for(iCard = 0; iCard < _out_cards.size(); iCard++) {
        for(iOut = 0; iOut < _out_cards[iCard].nOutput; iOut++) {
            if(id == _out_cards[iCard].output_id[iOut]) {
                if(status) {
                    _out_cards[iCard].cardData |= (1 << iOut);
                } else {
                    _out_cards[iCard].cardData &= ~(1 << iOut);
                }
                iTry = 0;
                do {
                    _relayError = i2c_write_uint8(_out_cards[iCard].card.address, REG_OUTPUT_DATA,
                                _out_cards[iCard].cardData);
                    if(_relayError == 0) {
                        DEBUG_MSG_P(PSTR("[CARD] Send 0x%02X to card %u: OK!\n"),
                                    _out_cards[iCard].cardData, iCard);
                        break;
                    } else {
                        iTry++;
                        DEBUG_MSG_P(PSTR("[CARD] Send 0x%02X to card %u, error: %u!\n"),
                                    _out_cards[iCard].cardData, iCard, _relayError);
                    }
                } while(iTry < 5);
                return;
            }
        }
    }
    DEBUG_MSG_P(PSTR("[CARD] Relay id %d NOT found to set status.\n"), id);
}

#endif