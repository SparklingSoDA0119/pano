#include "PanoPlateDetectorSetup.h"

#include <rapidjson/stringbuffer.h>
#include <rapidjson/prettywriter.h>
#include <rapidjson/istreamwrapper.h>
#include <rapidjson/ostreamwrapper.h>

#include <fstream>

namespace pano
{
	PdAreaInfo::PdAreaInfo()
		: _useName("use"), _use(false)
		, _areaCoordName("coord"), _stCoord(AREA_COORD_S())
	{

	}

	PdAreaInfo::PdAreaInfo(const PdAreaInfo& info)
		: PdAreaInfo()
	{
		*this = info;
	}

	PdAreaInfo::~PdAreaInfo()
	{

	}

	RET_E PdAreaInfo::parseObjectValue(const rapidjson::Value& value)
	{
		return RET_E::SUCCESS;
	}

	PdAreaInfo& PdAreaInfo::operator=(const PdAreaInfo& info)
	{
		_use = info._use;
		_stCoord = info._stCoord;

		return *this;
	}

	/// //////////////////////////////////////////////////////////////////////////////////////////////////

	PpdSetup::PpdSetup()
		: _totalAreaCntName("total_area_cnt"), _totalAreaCnt(0)
		, _areaInfosName("area_infos"), _vAreaInfos{}
	{
		_vAreaInfos.clear();
	}

	PpdSetup::PpdSetup(const PpdSetup& setup)
		: PpdSetup()
	{
		*this = setup;
	}

	PpdSetup::~PpdSetup()
	{

	}

	RET_E PpdSetup::readFile(const std::string& file)
	{
		std::lock_guard<std::mutex> al(_mutex);

		if (file.empty()) {
			return RET_E::FAILURE;
		}

		std::ifstream ifs(file);
		if (!ifs.is_open()) {
			return RET_E::FAILURE;
		}

		rapidjson::IStreamWrapper isw(ifs);
		_doc.ParseStream(isw);

		if (_doc.HasParseError()) {
			_doc.SetObject();
			return RET_E::FAILURE;
		}

		RET_E eRet = getMemberValues();
		if (eRet != RET_E::SUCCESS) {
			return eRet;
		}

		return eRet;
	}

	RET_E PpdSetup::getMemberValues()
	{
		RET_E eRet = RET_E::SUCCESS;

		eRet = parseTotalAreaCnt();
		if (eRet != RET_E::SUCCESS) {
			return eRet;
		}

		if (_totalAreaCnt) {
			eRet = parseAreaInfos();
			if (eRet != RET_E::SUCCESS) {
				return eRet;
			}
		}

		return eRet;
	}

	RET_E PpdSetup::parseTotalAreaCnt()
	{
		if (_doc.HasMember(_totalAreaCntName)) {
			if (_doc[_totalAreaCntName].IsInt()) {
				_totalAreaCnt = _doc[_totalAreaCntName].GetInt();
			}
			else {
				return RET_E::FAILURE;
			}
		}
		else {
			return RET_E::FAILURE;
		}

		return RET_E::SUCCESS;
	}

	RET_E PpdSetup::parseAreaInfos()
	{
		if (!_doc.HasMember(_areaInfosName)) {
			return RET_E::FAILURE;
		}

		if (!_doc[_areaInfosName].IsArray()) {
			return RET_E::FAILURE;
		}


		_vAreaInfos.clear();
		for (int32_t i = 0; i < _totalAreaCnt; i++) {
			if (_doc[_areaInfosName][i].IsObject()) {
				//rapidjson::Value objValue = _doc[_areaInfosName][i].GetObject();
				PdAreaInfo areaInfo;
				
				areaInfo.setUse(_doc[_areaInfosName][i][areaInfo.useName()].GetBool());
				AREA_COORD_S stCoord;
				
				stCoord.x = _doc[_areaInfosName][i][areaInfo.areaCoordName()][0].GetInt();
				stCoord.y = _doc[_areaInfosName][i][areaInfo.areaCoordName()][1].GetInt();
				stCoord.w = _doc[_areaInfosName][i][areaInfo.areaCoordName()][2].GetInt();
				stCoord.h = _doc[_areaInfosName][i][areaInfo.areaCoordName()][3].GetInt();

				areaInfo.setCoord(stCoord);

				_vAreaInfos.push_back(areaInfo);
			}
		}

		return RET_E::SUCCESS;
	}

	PpdSetup& PpdSetup::operator=(const PpdSetup& setup)
	{
		_totalAreaCnt = setup._totalAreaCnt;

		_vAreaInfos.clear();
		for (int32_t i = 0; i < _totalAreaCnt; i++) {
			_vAreaInfos.push_back(setup._vAreaInfos[i]);
		}

		return *this;
	}
}