#ifndef _PANO_PPD_CONST_H_
#define _PANO_PPD_CONST_H_

#include <iostream>
#include <stdio.h>

#include <opencv2/opencv.hpp>

namespace pano
{
	#define MAX_AREA_COUNT			14
	#define CLIENT_SOCKET_SERVER	"/tmp/composite_server"
	#define CLIENT_LOCAL_FILE		"/tmp/pgs"
	#define PGS_SERVER_FILE			"/tmp/pgs_server"
	#define RCV_BUFFER_SIZE			1024 * 512
	#define SND_BUFFER_SIZE			4096
	#define PGS_CONFIG_FILE			"/configs/composite_server/pgs.json"
	#define YUV_CAPTURE_FILE		"/dev/shm/captureParking"
	#define MAX_EMPTY_CHECK_DELAY	3
	#define MAX_FULL_CHECK_DELAY	5

	enum class RET_E
	{
		SUCCESS = 0,
		FAILURE,
	};
}	// namespace pano

#endif	// _PANO_PPD_CONST_H_