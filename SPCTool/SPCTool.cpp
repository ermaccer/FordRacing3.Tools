// SPCTool.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <fstream>
#include <iostream>
#include <vector>
#include "sound.h"
#include <filesystem>

enum eModes {
	MODE_EXPORT = 1,
	MODE_IMPORT

};

int main(int argc, char* argv[])
{
	if (argc == 1) {
		std::cout << "SPCTool - work with Ford Racing 3 sound archive by ermaccer\n"
			<< "Usage: spcotol <params> <file>\n"
			<< "    -e              Export .spc to .wav\n"
		//	<< "    -i             todo\n"
		//	<< "    -l             todo\n"
			<< "    -o              Specifies a folder for extraction/output file\n";
		return 1;
	}


	int mode = 0;
	std::string o_param;
	std::string l_param;
	// params
	for (int i = 1; i < argc - 1; i++)
	{
		if (argv[i][0] != '-' || strlen(argv[i]) != 2) {
			return 1;
		}
		switch (argv[i][1])
		{
		case 'e': mode = MODE_EXPORT;
			break;
		case 'i': mode = MODE_IMPORT;
			break;
		case 'o':
			i++;
			o_param = argv[i];
			break;
		case 'l':
			i++;
			l_param = argv[i];
			break;
		default:
			std::cout << "ERROR: Param does not exist: " << argv[i] << std::endl;
			break;
		}
	}


	if (mode == MODE_EXPORT)
	{
		std::ifstream pFile(argv[argc - 1], std::ifstream::binary);

		if (!pFile)
		{
			std::cout << "ERROR: Could not open: " << argv[argc - 1] << "!" << std::endl;
			return 1;
		}

		spc_header header;
		pFile.read((char*)&header, sizeof(spc_header));

		if (header.entries <= 0)
		{
			std::cout << "ERROR: Invalid amount of entries in " << argv[argc - 1] << "!" << std::endl;
			return 1;
		}

		std::vector<spc_entry> vSoundData;

		for (int i = 0; i < header.entries; i++)
		{
			spc_entry sound;
			pFile.read((char*)&sound, sizeof(spc_entry));
			vSoundData.push_back(sound);
		}

		if (!o_param.empty())
		{
			if (!std::filesystem::exists(o_param))
				std::filesystem::create_directory(o_param);
			std::filesystem::current_path(o_param);
		}


		// extract

		int baseOffset = (int)pFile.tellg();

		for (int i = 0; i < header.entries; i++)
		{
			pFile.seekg(baseOffset + vSoundData[i].baseOffset, pFile.beg);
			std::string name = vSoundData[i].name;
			name += ".wav";

			std::cout << "Extracting: " << name.c_str() << std::endl;


			std::unique_ptr<char[]> dataBuff = std::make_unique<char[]>(vSoundData[i].size);
			pFile.read(dataBuff.get(), vSoundData[i].size);

			std::ofstream oFile(name, std::ofstream::binary);
			oFile.write(dataBuff.get(), vSoundData[i].size);
		}
		std::cout << "Finished." << std::endl;
	}

	return 0;
}

