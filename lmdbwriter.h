#include "wacs-config.h"

/**
  * Return:  0- success
  *          1- can not listen port
  *          2- invalid nano socket URL
  *          3- buffer allocation error
  *          4- send error, re-open 
  */
int run
(
	WacsConfig *config
);

/**
  * Return 0- success
  *        1- config is not initialized yet
  */
int stop
(
	WacsConfig *config
);

int reload(
	WacsConfig *config
);
