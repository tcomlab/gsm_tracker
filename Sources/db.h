/*
 * db.h
 *
 *  Created on: 25.10.2010
 *      Author: Admin
 */

#ifndef DB_H_
#define DB_H_
#include "system.h"

void save_geo_in_db(SGPS *gps);
void save_io_in_db(SIO *io);
void init_database (void);
#endif /* DB_H_ */
