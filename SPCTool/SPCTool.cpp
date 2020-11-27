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
	MODE_CREATE

};

int main(int argc, char* argv[])
{
	if (argc == 1) {
		std::cout << "SPCTool - work with Ford Racing 3 sound archive by ermaccer\n"
			<< "Usage: spcotol <params> <file>\n"
			<< "    -e              Export .spc to .wav\n"
			<< "    -c              Create .spc from list file\n"
			<< "    -l              Specifies list file\n"
			<< "    -o              Specifies a folder for extraction/output file\n"
			<< "Example:\n"
			<< "spctool -e -o ui ui.spc | Extracts ui.spc to folder ui\n"
			<< "spctool -c -o newui.spc -l !ui.txt ui | Creates newui.spc from folder ui and using list !ui.txt\n"
			<< "List file must be in the same folder as .wavs!\n";
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
		case 'c': mode = MODE_CREATE;
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

		// generate list

		std::string listName = "!";
		listName += argv[argc - 1];
		listName.replace(listName.length() - 4, 4, ".txt");

		std::ofstream oList(listName, std::ofstream::binary);

		oList << "; generated using spctool by ermaccer" << std::endl;
		oList << "; name (32 max)\n; params (9)" << std::endl;
		for (int i = 0; i < header.entries; i++)
		{
			std::string name = vSoundData[i].name;
			oList << name.c_str() << " ";

			for (int a = 0; a < 9; a++)
				oList << vSoundData[i].params[a] << " ";

			oList << std::endl;

		}

		std::cout << "INFO: List file saved as: " << listName.c_str() << std::endl;

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
	if (mode == MODE_CREATE)
	{
		if (!std::filesystem::exists(argv[argc - 1]))
		{
			std::cout << "ERROR: Folder does not exist: " << argv[argc - 1] << "!" << std::endl;
			return 1;
		}

		if (l_param.empty())
		{
			std::cout << "ERROR: List file not specified!" << std::endl;
			return 1;
		}

		std::vector<spc_entry> vSoundData;
		std::vector<std::string> vNames;
		// write 


		std::string output = argv[argc - 1];
		output += ".spc";

		if (!o_param.empty()) output = o_param;

		std::ofstream oFile(output, std::ofstream::binary);



		std::filesystem::current_path(argv[argc - 1]);

		FILE* pList = fopen(l_param.c_str(), "rb");

		if (!pList)
		{
			std::cout << "ERROR: Failed to open list file: " << l_param.c_str() << "!" << std::endl;
			return 1;
		}

		char szLine[2048];
		while (fgets(szLine, sizeof(szLine), pList))
		{
			// check if comment or empty line
			if (szLine[0] == ';' || szLine[0] == '#' || szLine[0] == '\n')
				continue;
			char name[256];
			if (sscanf(szLine, "%s", &name) == 1)
			{

				int params[9];

				sscanf(szLine, "%s %d %d %d %d %d %d %d %d %d", &name, &params[0],
					&params[1],
					&params[2],
					&params[3],
					&params[4],
					&params[5],
					&params[6],
					&params[7],
					&params[8]
				);
				
				spc_entry spc;
				sprintf(spc.name, name);

				spc.params[0] = params[0];
				spc.params[1] = params[1];
				spc.params[2] = params[2];
				spc.params[3] = params[3];
				spc.params[4] = params[4];
				spc.params[5] = params[5];
				spc.params[6] = params[6];
				spc.params[7] = params[7];
				spc.params[8] = params[8];

				vSoundData.push_back(spc);
				std::string str = name;
				str = str.substr(0, 32);
				vNames.push_back(str);

			}
		
		}


		int baseOffset = 0;
		for (int i = 0; i < vSoundData.size(); i++)
		{
			vNames[i] += ".wav";
			if (std::filesystem::exists(vNames[i]))
			{
				vSoundData[i].size = (int)std::filesystem::file_size(vNames[i]);
				vSoundData[i].baseOffset = baseOffset;
				baseOffset += vSoundData[i].size;
			}
		}

		// write

		int size = vSoundData.size();
		oFile.write((char*)&size, sizeof(int));

		// entries
		for (int i = 0; i < vSoundData.size(); i++)
		{
			oFile.write((char*)&vSoundData[i], sizeof(spc_entry));
		}

		// file data

		for (int i = 0; i < vSoundData.size(); i++)
		{
			int size = vSoundData[i].size;

			std::ifstream pFile(vNames[i], std::ifstream::binary);

			if (!pFile)
			{
				std::cout << "ERROR: Could not open: " << vNames[i] << "!" << std::endl;
				return 1;
			}

			std::cout << "Processing: " << vNames[i] << std::endl;

			std::unique_ptr<char[]> dataBuff = std::make_unique<char[]>(size);
			pFile.read(dataBuff.get(), size);
			oFile.write(dataBuff.get(), size);
		}

		std::cout << "Saved as: " << output.c_str() << std::endl;
		std::cout << "Finished." << std::endl;




	}

	return 0;
}

