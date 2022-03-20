/**********************************************************************
*
* Copyright (C)2015-2022 Maxwell Analytica. All rights reserved.
*
* @file filecarver.cpp
* @brief 
* @details 
* @author Maxwell
* @version 1.0.0
* @date 2018-15-09 22:41:27.492
*
**********************************************************************/
#include "filecarver.h"

FileCarver::FileCarver()
{
	extension_ = "";
	algorithm_ = "";
	developer_id_ = 0;
	truncate_size_ = 0;
	carved_file_info_ = std::make_shared<CarvedFileInfo>();
	logic_tuple_ = std::make_tuple(LT_None, LT_None, LT_None);
	//
	initialize();
}

FileCarver::~FileCarver()
{

}

uint16_t FileCarver::WD_SECTOR_SIZE = 512;

void FileCarver::initialize()
{
	carver_status_ = CS_Init;
	//
	carved_file_info_->size = 0;
	carved_file_info_->block_count = 0;
	carved_file_info_->start_blockno = 0;
}

uint64_t FileCarver::getDeveloperId() const
{
	return developer_id_;
}

std::string FileCarver::getExtension() const
{
	return extension_;
}

CarverStatus FileCarver::getCarverStatus() const
{
	return carver_status_;
}

LogicType FileCarver::logicType(std::string& logic)
{
	std::string symbol = MaUtil::lower(logic);
	if (symbol == "&" || symbol == "&&" || symbol == "and")
		return LogicType::LT_And;
	else if (symbol == "|" || symbol == "||" || symbol == "or")
		return LogicType::LT_Or;
	else if (symbol == "!" || symbol == "not")
		return LogicType::LT_Not;
	//
	return LogicType::LT_None;
}

std::shared_ptr<CarvedFileInfo> FileCarver::getCarvedFileInfo() const
{
	return carved_file_info_;
}

int32_t FileCarver::setCharacteristics(frjson::object_t object)
{
	if (object.empty())
		return -1;
	
	try
	{
		auto iter = object.find("extension");// extension
		if (iter == object.end())
			return -1;
		extension_ = iter->second.get<std::string>();
		
		iter = object.find("developerId");	// developer ID
		if (iter != object.end())
			developer_id_ = iter->second.get<uint64_t>();
		
		iter = object.find("algorithm");	// optional
		if (iter != object.end())
			algorithm_ = iter->second.get<std::string>();
		
		iter = object.find("truncate");		// optional
		if (iter != object.end())
			truncate_size_ = iter->second.get<int64_t>();
		
		frjson::object_t name_object;
		iter = object.find("name");			// optional
		if (iter != object.end())
			name_object = iter->second.get<frjson::object_t>();
		
		iter = object.find("header");		// need
		if (iter == object.end())
			return -1;
		auto header_object = iter->second.get<frjson::object_t>();
		
		frjson::object_t body_object;
		iter = object.find("body");			// optional
		if (iter != object.end())
			body_object = iter->second.get<frjson::object_t>();
		
		iter = object.find("footer");		// need
		if (iter == object.end())
			return -1;
		auto footer_object = iter->second.get<frjson::object_t>();
		if (!name_object.empty())
		{
			name_info_ = std::make_shared<NameInfo>();
			name_info_->offset = name_object.at("offset").get<int32_t>();
			name_info_->size = name_object.at("size").get<int32_t>();
			name_info_->padding = name_object.at("padding").get<int32_t>();
		}
		if (!header_object.empty())
		{
			std::string logic = header_object.at("logic").get<std::string>();
			std::get<0>(logic_tuple_) = logicType(logic);
			frjson::array_t character_array = header_object.at("characters").get<frjson::array_t>();
			for (auto& character_object : character_array)
			{
				auto character = std::make_shared<CharacterInfo>();
				character->hex = character_object.at("hex").get<bool>();
				character->size = character_object.at("size").get<int32_t>();
				character->amphibious.offset = character_object.at("offset").get<int32_t>();
				std::string context = character_object.at("context").get<std::string>();
				if (character->hex)
				{
					const int32_t size = context.length() > 64 ? 64 : context.length();
					for (int32_t pos = 0; pos < size; pos += 2)
					{
						int32_t count = (size - pos) > 1 ? 2 : 1;
						std::string el = "0x" + context.substr(pos, count);
						character->character[pos / 2] = stoi(el, 0, 16);
					}
				}
				else
				{
					memcpy(character->character, context.c_str(), context.length() > 64 ? context.length() : 64);
				}
				header_vector_.emplace_back(character);
			}
		}
		if (!body_object.empty())
		{
			std::string logic = body_object.at("logic").get<std::string>();
			std::get<1>(logic_tuple_) = logicType(logic);
			frjson::array_t character_array = body_object.at("characters").get<frjson::array_t>();
			for (auto& character_object : character_array)
			{
				auto character = std::make_shared<CharacterInfo>();
				character->hex = character_object.at("hex").get<bool>();
				character->size = character_object.at("size").get<int32_t>();
				character->amphibious.offset = character_object.at("offset").get<int32_t>();
				std::string context = character_object.at("context").get<std::string>();
				if (character->hex)
				{
					const int32_t size = context.length() > 64 ? 64 : context.length();
					for (int32_t pos = 0; pos < size; pos += 2)
					{
						int32_t count = (size - pos) > 1 ? 2 : 1;
						std::string el = "0x" + context.substr(pos, count);
						character->character[pos / 2] = stoi(el, 0, 16);
					}
				}
				else
				{
					memcpy(character->character, context.c_str(), context.length() > 64 ? context.length() : 64);
				}
				body_vector_.emplace_back(character);
			}
		}
		if (!footer_object.empty())
		{
			std::string logic = footer_object.at("logic").get<std::string>();
			std::get<2>(logic_tuple_) = logicType(logic);
			frjson::array_t character_array = footer_object.at("characters").get<frjson::array_t>();
			for (auto& character_object : character_array)
			{
				auto character = std::make_shared<CharacterInfo>();
				character->hex = character_object.at("hex").get<bool>();
				character->size = character_object.at("size").get<int32_t>();
				character->amphibious.padding = character_object.at("padding").get<int32_t>();
				std::string context = character_object.at("context").get<std::string>();
				if (character->hex)
				{
					const int32_t size = context.length() > 64 ? 64 : context.length();
					for (int32_t pos = 0; pos < size; pos += 2)
					{
						int32_t count = (size - pos) > 1 ? 2 : 1;
						std::string el = "0x" + context.substr(pos, count);
						character->character[pos / 2] = stoi(el, 0, 16);
					}
				}
				else
				{
					memcpy(character->character, context.c_str(), context.length() > 64 ? context.length() : 64);
				}
				footer_vector_.emplace_back(character);
			}
		}
	}
	catch (const std::exception& e)
	{
		std::cout << "parse characteristic exception: " << e.what() << std::endl;
	}
	
	return 0;
}

int32_t FileCarver::analyzeHeader(std::shared_ptr<ClusterPackage> package)
{
	bool matched = true;
	if (std::get<0>(logic_tuple_) == LT_And)
	{
		for (auto& info : header_vector_)
		{
			if (memcmp(info->character, package->Buffer + info->amphibious.offset, info->size) != 0)
			{
				matched = false;
				break;
			}
		}
	}
	else if (std::get<0>(logic_tuple_) == LT_Or)
	{
		matched = false;
		for (auto& info : header_vector_)
		{
			if (memcmp(info->character, package->Buffer + info->amphibious.offset, info->size) == 0)
			{
				matched = true;
				break;
			}
		}
	}
	else if (std::get<0>(logic_tuple_) == LT_Not)
	{
		matched = false;
		for (auto& info : header_vector_)
		{
			if (memcmp(info->character, package->Buffer + info->amphibious.offset, info->size) == 0)
				break;
		}
	}
	
	if (matched)
	{
		if (name_info_ != nullptr)
		{
			uint16_t name_size = 0;
			memcpy(&name_size, package->Buffer + name_info_->offset, name_info_->size);
			if (name_size < 0x100)
			{
				auto pBuffer = new char[name_size];
				memcpy(pBuffer, package->Buffer + name_info_->offset + name_info_->size + name_info_->padding, name_size);
				carved_file_info_->base_name.assign(pBuffer, name_size);
				size_t pos = carved_file_info_->base_name.find("\\");
				if (pos > 0)
					carved_file_info_->base_name = carved_file_info_->base_name.substr(0, pos);
				delete[] pBuffer;
			}
		}
		//
		carved_file_info_->start_blockno = package->BlockNumber;
		carver_status_ = CS_Header;
	}
	//
	return -1;
}

int32_t FileCarver::analyzeBody(std::shared_ptr<ClusterPackage> package)
{
	return -1;
}

int32_t FileCarver::analyzeFooter(std::shared_ptr<ClusterPackage> package)
{
	if (package->BlockNumber < carved_file_info_->start_blockno)
		return -1;
	
	int32_t offset = 0;
	std::shared_ptr<CharacterInfo> character_info;
	if (std::get<2>(logic_tuple_) == LT_And)
	{
		for (auto& info : footer_vector_)
		{
			offset = MaUtil::BoyerMoore(package->Buffer, WD_BLOCK_SIZE, (char*)info->character, info->size);
			if (offset < 0)
				return -1;
			character_info = info;
		}
	}
	else if (std::get<2>(logic_tuple_) == LT_Or)
	{
		for (auto& info : header_vector_)
		{
			offset = MaUtil::BoyerMoore(package->Buffer, WD_BLOCK_SIZE, (char*)info->character, info->size);
			if (offset < 0)
				return -1;
			character_info = info;
			break;
		}
	}
	else if (std::get<2>(logic_tuple_) == LT_Not)
	{
		for (auto& info : header_vector_)
		{
			offset = MaUtil::BoyerMoore(package->Buffer, WD_BLOCK_SIZE, (char*)info->character, info->size);
			if (offset >= 0)
				return -1;
			character_info = info;
		}
	}
	
	if (character_info == nullptr)
		return -1;
	
	uint16_t block_count = (offset + character_info->size + character_info->amphibious.padding) / FileCarver::WD_SECTOR_SIZE + 1;
	uint16_t remain_count = (offset + character_info->size + character_info->amphibious.padding) % FileCarver::WD_SECTOR_SIZE;
	
	carved_file_info_->block_count = package->BlockNumber - carved_file_info_->start_blockno + block_count;
	carved_file_info_->size = FileCarver::WD_SECTOR_SIZE * (carved_file_info_->block_count - 1) + remain_count;
	
	carver_status_ = CS_Footer;
	
	return 0;
}

int32_t FileCarver::truncate(std::shared_ptr<ClusterPackage> package)
{
	if (truncate_size_ <= 0)
		return -1;
	
	if (package->BlockNumber < carved_file_info_->start_blockno)
		return -1;
	
	int64_t block_count = package->BlockNumber - carved_file_info_->start_blockno;
	if (FileCarver::WD_SECTOR_SIZE * block_count < truncate_size_)
		return -1;
	
	carved_file_info_->block_count = package->BlockNumber - carved_file_info_->start_blockno + 1;
	carved_file_info_->size = FileCarver::WD_SECTOR_SIZE * carved_file_info_->block_count;
	
	carver_status_ = CS_Footer;
	
	return 0;
}