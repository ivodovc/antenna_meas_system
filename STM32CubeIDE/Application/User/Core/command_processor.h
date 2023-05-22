/*
 * command_processor.h
 *
 *  Created on: May 5, 2023
 *      Author: IvkoPivko
 */

#ifndef APPLICATION_USER_CORE_COMMAND_PROCESSOR_H_
#define APPLICATION_USER_CORE_COMMAND_PROCESSOR_H_

typedef enum{AMS_NONE, AMS_SWEEP, AMS_SWEEP_CONT, AMS_REGISTER,
			AMS_VERSION, AMS_SINGLE, AMS_HOWAREYOU,
			AMS_STOP, AMS_CHECK}
command_t;

#define MAX_ARG_LEN 10


#endif /* APPLICATION_USER_CORE_COMMAND_PROCESSOR_H_ */
