/**
 *   @file: ata_demo.h
 *
 *   @date: Jul 5, 2017
 * @author: Mateusz Midor
 */

#ifndef SRC__DEMOS_ATA_DEMO_H_
#define SRC__DEMOS_ATA_DEMO_H_

#include <memory>
#include "AtaDriver.h"

namespace demos {

void ata_demo(std::shared_ptr<drivers::AtaPrimaryBusDriver> ata_primary_bus);

}

#endif /* SRC__DEMOS_ATA_DEMO_H_ */
