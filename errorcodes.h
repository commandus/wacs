/*
 * errorcodes.h
 *
 */

#ifndef ERRORCODES_H_
#define ERRORCODES_H_

#include <string>

#define MSG_OK									"OK"
#define MSG_FAILED								"failed"
#define MSG_START								"Start"
#define MSG_STOP								"Stopped.."
#define MSG_DONE								"Done"
#define MSG_INTERRUPTED							"Interrupted"
#define MSG_TERMINATE							"Terminate.. "
#define MSG_TERMINATED							"Terminated"
#define MSG_RELOAD_CONFIG_REQUEST				"Reload config signal requested "
#define MSG_RELOAD_BEGIN						"Reload.. "
#define MSG_RELOAD_END							"Reloaded "
#define MSG_SIGNAL								"Signal "
#define MSG_DAEMONIZE							"Start as daemon, use syslog"
#define MSG_PROCESSED							"successfully processed"
#define MSG_EOF									"End of file detected after record "
#define MSG_LOOP_EXIT							"Event loop exit"
#define MSG_CONNECTED_TO						"Connected to: "
#define MSG_SENT								"Sent bytes: "
#define MSG_RECEIVED							"Received bytes: "
#define MSG_ADDED								"added successfully "
#define MSG_PUT_ERROR							"put entry error "
#define MSG_NN_BIND_SUCCESS						"nanomsg socket bind successfully to "
#define MSG_NN_SENT_SUCCESS						"nanomsg socket sent data successfully to "
#define MSG_PACKET_REJECTED						"packet rejected: "
#define MSG_CHILD_WAITING						"Child process terminating, waiting  "
#define MSG_CHILD_TERMINATED					"Child process terminated "
#define MSG_EMPTY_PACKET						"Empty packet received"
#define MSG_SHUTDOWN							"Shutdown "
#define MSG_BIND_SUCCESSFULLY					"Socket binded successfully to "

#define ERR_OK									0
#define ERRCODE_COMMAND							-1
#define ERRCODE_PARSE_COMMAND					-2
#define ERRCODE_WRONG_PARAM						-3

#define ERRCODE_LMDB_TXN_BEGIN					-10
#define ERRCODE_LMDB_TXN_COMMIT					-11
#define ERRCODE_LMDB_OPEN						-12
#define ERRCODE_LMDB_CLOSE						-13
#define ERRCODE_LMDB_PUT						-14
#define ERRCODE_LMDB_PUT_PROBE					-15
#define ERRCODE_LMDB_GET						-16

#define ERRCODE_NN_SOCKET						-20
#define ERRCODE_NN_CONNECT	 					-21
#define ERRCODE_NN_BIND		 					-22
#define ERRCODE_NN_SUBSCRIBE					-23
#define ERRCODE_NN_SHUTDOWN 					-24
#define ERRCODE_NN_RECV							-25
#define ERRCODE_NN_SEND							-26
#define ERRCODE_NN_SET_SOCKET_OPTION			-27
#define ERRCODE_NN_FREE_MSG						-28
#define ERRCODE_NN_ACCEPT						-29

#define ERRCODE_STOP							-40
#define ERRCODE_NO_CONFIG						-41
#define ERRCODE_NO_MEMORY						-42
#define ERRCODE_NOT_IMPLEMENTED					-43

#define ERRCODE_SOCKET_SEND						-50
#define ERRCODE_GET_ADDRINFO                    -51
#define ERRCODE_SOCKET_CREATE					-52
#define ERRCODE_SOCKET_SET_OPTIONS				-53
#define ERRCODE_SOCKET_BIND						-54
#define ERRCODE_SOCKET_CONNECT                  -55
#define ERRCODE_SOCKET_LISTEN					-56
#define ERRCODE_SOCKET_READ						-57
#define ERRCODE_SOCKET_WRITE					-58

#define ERRCODE_FORK							-60
#define ERRCODE_EXEC							-61
#define ERRCODE_CONFIG							-62
#define ERRCODE_WSA_ERROR						-63

#define ERRCODE_NO_NOBILE_SUBSCRIBERS			-70

#define ERRCODE_DATABASE_NO_CONNECTION			-80
#define ERRCODE_DATABASE_STATEMENT_FAIL			-81

#define ERR_COMMAND								"Invalid command line options or help requested. "
#define ERR_PARSE_COMMAND						"Error parse command line options, possible cause is insufficient memory. "
#define ERR_LMDB_TXN_BEGIN						"Can not begin LMDB transaction "
#define ERR_LMDB_TXN_COMMIT						"Can not commit LMDB transaction "
#define ERR_LMDB_OPEN							"Can not open database file "
#define ERR_LMDB_CLOSE							"Can not close database file "
#define ERR_LMDB_PUT							"Can not put LMDB "
#define ERR_LMDB_PUT_PROBE						"Can not put LMDB probe "
#define ERR_LMDB_GET							"Can not get LMDB "

#define ERR_NN_SOCKET							"Can create nanomsg socket "
#define ERR_NN_CONNECT							"Can not connect nanomsg socket to the IPC url "
#define ERR_NN_BIND								"Can not bind nanomsg socket to the IPC url "
#define ERR_NN_SUBSCRIBE						"Can not subscribe nanomsg socket to the IPC url "
#define ERR_NN_SHUTDOWN							"Can not shutdown nanomsg socket "
#define ERR_NN_RECV								"Receive nanomsg error "
#define ERR_NN_SEND								"Send nanomsg socket error "
#define ERR_NN_ACCEPT							"nanomsg accept socket error: "
#define ERR_NN_SET_SOCKET_OPTION				"nanomsg set socket option error: "
#define ERR_NN_FREE_MSG							"nanomsg free message error: "

#define ERR_STOP								"Can not stop"
#define ERR_NO_CONFIG							"No config provided"

#define ERR_NO_MEMORY							"Can not allocate buffer size "
#define ERR_DECODE_MESSAGE						"Error decode message "
#define ERR_DECODE_MESSAGE_OR_EOF				"Error decode message or EOF, exit "

#define ERR_SOCKET_SEND							"Error socket send "
#define ERR_NOT_IMPLEMENTED					    "Not implemented"

#define ERR_GET_ADDRINFO                        "Address resolve error: "
#define ERR_SOCKET_CREATE 						"Socket create error: "
#define ERR_SOCKET_SET_OPTIONS					"Socket set options error"
#define ERR_SOCKET_BIND							"Socket bind error "
#define ERR_SOCKET_CONNECT                      "Socket connect error "
#define ERR_SOCKET_LISTEN						"Socket listen error "
#define ERR_SOCKET_READ							"Socket read error "
#define ERR_SOCKET_WRITE						"Socket write error "

#define ERR_FORK								"Can not fork "
#define ERR_EXEC								"Can not execute "
#define ERR_CONFIG								"Load config file error "
#define ERR_INTERRUPTED							"Interruped "

#define ERR_TOO_SMALL							"Insufficient buffer space "
#define ERR_WSA_ERROR 							"WSAStartup error %d "
#define ERR_WRONG_PARAM							"Wrong parameter "


#define ERR_DATABASE_NO_CONNECTION				"PostgreSQL connection failed "
#define ERR_DATABASE_STATEMENT_FAIL				"PostgreSQL statement failed "

std:: string getErrorDescription(int code);

#endif /* ERRORCODES_H_ */
