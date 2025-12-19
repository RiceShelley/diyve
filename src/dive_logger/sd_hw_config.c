#include <string.h>
#include "my_debug.h"
#include "hw_config.h"
#include "ff.h"
#include "diskio.h"
#include "dive_computer_pins.h"

// Hardware Configuration of SPI "objects"
static spi_t spis[] = {
    {
        .hw_inst = SD_SPI_PORT,
        .miso_gpio = SD_SPI_MISO_PIN,
        .mosi_gpio = SD_SPI_MOSI_PIN,
        .sck_gpio = SD_SPI_SCK_PIN,
        .baud_rate = 12500 * 1000,  
    }
};

// Hardware Configuration of the SD Card "objects"
static sd_card_t sd_cards[] = {
    {
        .pcName = "0:",
        .spi = &spis[0],
        .ss_gpio = SD_SPI_SS_PIN,
        .use_card_detect = false
    }
};

size_t sd_get_num() {
    return count_of(sd_cards);
}

sd_card_t *sd_get_by_num(size_t num) {
    if (num <= sd_get_num()) {
        return &sd_cards[num];
    } else {
        return NULL;
    }
}

size_t spi_get_num() {
    return count_of(spis);
}

spi_t *spi_get_by_num(size_t num) {
    if (num <= sd_get_num()) {
        return &spis[num];
    } else {
        return NULL;
    }
}
