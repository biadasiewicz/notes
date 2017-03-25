#include "archive_io.h"
#include "user.h"
#include "archive.h"
#include "errors.h"
#include <crypto++/modes.h>
#include <crypto++/aes.h>
#include <crypto++/filters.h>
#include <cereal/archives/xml.hpp>
#include <cerrno>
#include <cstring>
#include <fstream>
#include <streambuf>
#include <sstream>
#include <functional>
namespace fs = boost::filesystem;

namespace notes
{

fs::path
make_path_from_date
( std::string year,
  std::string month,
  std::string day )
{
	std::string filename;
	filename.reserve(year.size() + month.size() + day.size() + 2);
	filename += std::move(year) + "_";
	filename += std::move(month) + "_";
	filename += std::move(day);

	fs::path path = user_config.archive_path();
	path /= filename;

	return path;
}

fs::path make_path_from_date(time_t t)
{
	struct tm* date = localtime(&t);

	using std::to_string;
	return make_path_from_date(to_string(date->tm_year + 1900),
		to_string(date->tm_mon + 1), to_string(date->tm_mday));
}

template<typename TCereal_archive, typename TStream, typename TError> static
void generic_serialize(Archive& ar, fs::path const& path)
{
	try {
		if(path.empty()) {
			throw Empty_path_error();
		}

		TStream stream(path.string());
		if(!stream) {
			throw std::runtime_error(strerror(errno) +
				std::string(": ") + path.string());
		}

		TCereal_archive archive(stream);

		archive(ar);

	} catch(std::exception& e) {
		throw TError{ e.what() };
	}
}

static std::string to_encrypted_string(std::string const& text)
{
	try {
		byte key[ CryptoPP::AES::DEFAULT_KEYLENGTH ];
		byte iv[ CryptoPP::AES::BLOCKSIZE ];

		memset( key, 0x00, CryptoPP::AES::DEFAULT_KEYLENGTH );
		memset( iv, 0x00, CryptoPP::AES::BLOCKSIZE );

		CryptoPP::AES::Encryption aes_encryption(
			key, CryptoPP::AES::DEFAULT_KEYLENGTH);
		CryptoPP::CBC_Mode_ExternalCipher::Encryption cbc_encryption(
			aes_encryption, iv);

		std::string ciphertext;
		CryptoPP::StreamTransformationFilter stf_encryptor(cbc_encryption,
			new CryptoPP::StringSink(ciphertext));
		stf_encryptor.Put(reinterpret_cast<const unsigned char*>(
			text.c_str()), text.length());
		stf_encryptor.MessageEnd();

		return ciphertext;
	} catch(std::exception& e) {
		throw Archive_encryption_error(e.what());
	} catch(...) {
		throw Archive_encryption_error{};
	}
}

static std::string to_decrypted_string(std::string const& encrypted)
{
	try {
		byte key[ CryptoPP::AES::DEFAULT_KEYLENGTH ];
		byte iv[ CryptoPP::AES::BLOCKSIZE ];

		memset( key, 0x00, CryptoPP::AES::DEFAULT_KEYLENGTH );
		memset( iv, 0x00, CryptoPP::AES::BLOCKSIZE );

		CryptoPP::AES::Decryption aes_decryption(key,
			CryptoPP::AES::DEFAULT_KEYLENGTH);
		CryptoPP::CBC_Mode_ExternalCipher::Decryption cbc_decryption(
			aes_decryption, iv);

		std::string decrypted;
		decrypted.reserve(encrypted.length() + 1000);

		CryptoPP::StreamTransformationFilter stf_decryptor(cbc_decryption,
			new CryptoPP::StringSink(decrypted));
		stf_decryptor.Put(reinterpret_cast<const unsigned char*>(
			encrypted.c_str()), encrypted.size());
		stf_decryptor.MessageEnd();

		return decrypted;
	} catch(std::exception& e) {
		throw Archive_decryption_error(e.what());
	} catch(...) {
		throw Archive_decryption_error{};
	}
}

static std::string to_string(Archive& ar)
{
	std::ostringstream oss;
	{
		cereal::XMLOutputArchive oa(oss);
		oa(ar);
	}

	return oss.str();
}

static Archive from_string(std::string str)
{
	Archive ar;
	std::istringstream iss(std::move(str));
	{
		cereal::XMLInputArchive ia(iss);
		ia(ar);
	}

	return ar;
}

static void save(std::string text, boost::filesystem::path const& path)
{
	std::ofstream file(path.string(), std::ios::binary);
	if(!file) {
		throw std::runtime_error(
			std::string("failed to open file for save: ") +
			strerror(errno));
	}

	file << text;

	if(file.bad() || file.fail()) {
		throw std::runtime_error(
			std::string("error while saving file: ") +
			strerror(errno));
	}
}

static std::string load(boost::filesystem::path const& path)
{
	std::ifstream file(path.string(), std::ios::binary);
	if(!file) {
		throw std::runtime_error(
			std::string("failed to open file for load: ") +
			strerror(errno));
	}

	std::ostringstream oss;
	oss << file.rdbuf();

	if(oss.bad() || oss.fail()) {
		throw std::runtime_error(
			std::string("error while loading file: ") +
			strerror(errno));
	}

	return oss.str();
}

void save(Archive& ar, boost::filesystem::path const& path)
{
	try {
		auto str = to_string(ar);
		auto encrypted = to_encrypted_string(str);
		save(encrypted, path);
	} catch(std::exception& e) {
		throw Archive_save_error(e.what());
	} catch(...) {
		throw Archive_save_error{};
	}
}

void load(Archive& ar, boost::filesystem::path const& path)
{
	try {
		std::string encrypted = load(path);
		ar = from_string(to_decrypted_string(encrypted));
	} catch(std::exception& e) {
		throw Archive_load_error(e.what());
	} catch(...) {
		throw Archive_load_error{};
	}
}

}
