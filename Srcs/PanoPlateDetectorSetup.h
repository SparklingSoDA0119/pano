#ifndef _PANO_PLATE_DETECTOR_SETUP_H_
#define _PANO_PLATE_DETECTOR_SETUP_H_

#include "Const.h"

#define RAPIDJSON_HAS_STDSTRING		1
#include <rapidjson/document.h>

#include <mutex>
#include <vector>

namespace pano
{
	typedef struct area_coord_s
	{
		int32_t x, y, w, h;

		area_coord_s() : x(0), y(0), w(0), h(0) {}
		area_coord_s(int32_t xVal, int32_t yVal, int32_t wVal, int32_t hVal) : x(xVal), y(yVal), w(wVal), h(hVal) {}
		area_coord_s& operator=(const area_coord_s& stCoord)
		{
			x = stCoord.x;
			y = stCoord.y;
			w = stCoord.w;
			h = stCoord.h;
			return *this;
		}
	} AREA_COORD_S;

	class PdAreaInfo
	{
	public :
		PdAreaInfo();
		PdAreaInfo(const PdAreaInfo& info);
		~PdAreaInfo();

	private :
		std::string _useName; bool _use;
		std::string _areaCoordName; AREA_COORD_S _stCoord;

	public :
		const std::string& useName() const { return _useName; }
		const std::string& areaCoordName() const { return _areaCoordName; }
		RET_E parseObjectValue(const rapidjson::Value& value);

	public :
		void setUse(bool use) { _use = use; }
		void setCoord(const AREA_COORD_S& stCoord) { _stCoord = stCoord; }

	public :
		bool isUse() const { return _use; }
		const AREA_COORD_S& coord() const { return _stCoord; }
		
	public :
		PdAreaInfo& operator=(const PdAreaInfo& info);
	};	// class PdAreaInfo

	class PpdSetup
	{
	public :
		PpdSetup();
		PpdSetup(const PpdSetup& setup);
		~PpdSetup();

	private :
		rapidjson::Document _doc;
		std::mutex _mutex;

	private :
		std::string _totalAreaCntName; int32_t _totalAreaCnt;
		std::string _areaInfosName; std::vector<PdAreaInfo> _vAreaInfos;

	public :
		RET_E readFile(const std::string& file);
		const std::vector<PdAreaInfo>& areaInfos() const { return _vAreaInfos; }

	private :
		RET_E getMemberValues();
		RET_E parseTotalAreaCnt();
		RET_E parseAreaInfos();

	public :
		PpdSetup& operator=(const PpdSetup& setup);
	};
}

#endif	// _PANO_PLATE_DETECTOR_SETUP_H_