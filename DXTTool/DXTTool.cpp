// DXTTool.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <vector>
#include <fstream>
#include <iostream>
#include <string>
#include <filesystem>
#include "texture.h"

#include "zlib\zlib.h"

#pragma comment (lib, "zlib/zdll.lib" )
enum eModes {
	MODE_EXPORT = 1,
	MODE_CREATE,
	MODE_DECOMPRESS,
	MODE_COMPRESS

};

int main(int argc, char* argv[])
{
	if (argc == 1) {
		std::cout << "DXTTool - work with Ford Racing 3 texture archive by ermaccer\n"
			<< "Usage: dxttool <params> <file>\n"
			<< "    -e              Export .dxt to .dds\n"
			<< "    -c              Create .dxt from input folder\n"
			<< "    -d              Decompress file\n"
			<< "    -z				Compress file\n"
			<< "    -l              Specifies list file\n"
			<< "    -t              Specifies texture data file\n"
			<< "    -o              Specifies a folder for extraction\n"
			<< "List file must be in the same folder as texture files!\n";
		return 1;
	}

	int mode = 0;
	std::string o_param;
	std::string l_param;
	std::string t_param;
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
		case 'd': mode = MODE_DECOMPRESS;
			break;
		case 'z': mode = MODE_COMPRESS;
			break;
		case 'o':
			i++;
			o_param = argv[i];
			break;
		case 'l':
			i++;
			l_param = argv[i];
			break;
		case 't':
			i++;
			t_param = argv[i];
			break;
		default:
			std::cout << "ERROR: Param does not exist: " << argv[i] << std::endl;
			break;
		}
	}
	if (mode == MODE_DECOMPRESS)
	{
		std::ifstream pFile(argv[argc - 1], std::ifstream::binary);

		if (!pFile)
		{
			std::cout << "ERROR: Could not open: " << argv[argc - 1] << "!" << std::endl;
			return 1;
		}

		if (pFile)
		{
			int size = std::filesystem::file_size(argv[argc - 1]);
			int rawSize;
			pFile.read((char*)&rawSize, sizeof(int));


			std::unique_ptr<char[]> dataBuff = std::make_unique<char[]>(size);
			pFile.read(dataBuff.get(), size);


			// decompress data
			std::unique_ptr<char[]> uncompressedBuffer = std::make_unique<char[]>(rawSize);
			unsigned long uncompressedSize = rawSize;
			int zlib_output = uncompress((Bytef*)uncompressedBuffer.get(), &uncompressedSize,
				(Bytef*)dataBuff.get(), size);


			if (zlib_output == Z_MEM_ERROR)
			{
				std::cout << "ERROR: ZLIB: Out of memory!" << std::endl;
				return 1;
			}

			std::string output = "dec_";
			output += argv[argc - 1];

			std::ofstream oFile(output, std::ofstream::binary);
			
			oFile.write(uncompressedBuffer.get(), uncompressedSize);

			std::cout << "Decompressed file " << argv[argc - 1] << " as " << output.c_str() << "!" << std::endl;
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


		if (pFile)
		{
			int fileSize = std::filesystem::file_size(argv[argc - 1]);
			int textures;

			pFile.read((char*)&textures, sizeof(int));
				
			if (textures <= 0)
			{
				std::cout << "ERROR: Invalid amount of entries in " << argv[argc - 1] << "!" << std::endl;
				return 1;
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

			oList << "; generated using dxttool by ermaccer" << std::endl;
			oList << "; name\n; filename\n; params" << std::endl;

			pFile.seekg(sizeof(int), pFile.cur);

			int unkEntries;

			pFile.read((char*)&unkEntries, sizeof(int));

			pFile.seekg(0, pFile.beg);
			// seems to work fine with vehicle textures
			int dataToSkip = (((unkEntries + textures) * 1024) + 20);

			std::unique_ptr<char[]> textureBuff = std::make_unique<char[]>(dataToSkip);
			pFile.read(textureBuff.get(), dataToSkip);

			std::string tmpText = argv[argc - 1];
			tmpText.replace(tmpText.length() - 4, 4, ".tdata");
			tmpText.insert(0, "data_");

			std::ofstream textureData(tmpText, std::ofstream::binary);
			textureData.write(textureBuff.get(), dataToSkip);


			//pFile.seekg(dataToSkip, pFile.beg);

			for (int i = 0; i < textures + 1; i++)
			{
				// check if doesn't overflow
				if ((int)pFile.tellg() >= fileSize)
					break;

				unsigned int flags;
				pFile.read((char*)&flags, sizeof(int));
				int strLen;
				pFile.read((char*)&strLen, sizeof(int));

				std::unique_ptr<char[]> name = std::make_unique<char[]>(strLen);
				pFile.read(name.get(), strLen);

				int dxtType;
				pFile.read((char*)&dxtType, sizeof(int));

				int unk;
				pFile.read((char*)&unk, sizeof(int));

				float unk_float;
				pFile.read((char*)&unk_float, sizeof(float));

				int x;
				pFile.read((char*)&x, sizeof(int));

				int y;
				pFile.read((char*)&y, sizeof(int));

				int unk_int;
				pFile.read((char*)&unk_int, sizeof(int));

				unsigned int flags2;
				pFile.read((char*)&flags2, sizeof(int));
				int textureDataSize = 0;

				if (dxtType == 1 || dxtType == 2)
					textureDataSize = ((x * y) / 2);
				else if (dxtType == 3)
					textureDataSize = (x * y);

				std::unique_ptr<char[]> dxtBuff = std::make_unique<char[]>(textureDataSize);

				pFile.read(dxtBuff.get(), textureDataSize);

				//printf("Texture [%d]\n", i + 1);
				//printf("Name: %s\n", name.get());
				//printf("Flags: 0x%X 0x%X\n", flags, flags2);
				//printf("DXT Type: %d\n", dxtType);
				//printf("Size: %dx%d\n", x, y);



				std::string fileName = std::to_string(i) + "_";
				fileName += name.get();
				fileName += ".dds";

				std::cout << "Extracting: " << fileName.c_str() << std::endl;


				oList << name.get() << "\t" << fileName.c_str() << "\t" << unk << "\t" << std::dec <<  unk_int << "\t" << unk_float << "\t" << std::hex << flags << "\t" << flags2 << std::endl;

				char type;

				if (dxtType == 1 || dxtType == 2)
					type = '1';
				else if (dxtType == 3)
					type = '3';

				dds_header dds;
				dds.dwSize = (sizeof(dds_header) - sizeof(dds.magic));
				dds.dwFlags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH | DDSD_PIXELFORMAT | DDSD_MIPMAPCOUNT | DDSD_LINEARSIZE;
				dds.dwHeight = y;
				dds.dwWidth = x;
				dds.dwPitchOrLinearSize = 0;
				dds.dwDepth = 0;
				dds.dwMipMapCount = 0;
				dds.ddspf.dwSize = sizeof(DDS_PIXELFORMAT);
				dds.ddspf.dwFlags = DDPF_FOURCC;
				dds.ddspf.dwFourCC[0] = 'D';
				dds.ddspf.dwFourCC[1] = 'X';
				dds.ddspf.dwFourCC[2] = 'T';
				dds.ddspf.dwFourCC[3] = type;
				dds.ddspf.dwRGBBitCount = 256;
				dds.ddspf.dwRBitMask = 0x00000000;
				dds.ddspf.dwGBitMask = 0x00000000;
				dds.ddspf.dwBBitMask = 0x00000000;
				dds.ddspf.dwABitMask = 0x00000000;
				dds.dwCaps = DDSCAPS_COMPLEX | DDSCAPS_TEXTURE | DDSCAPS_MIPMAP;
				// no need
				dds.dwCaps2 = 0;
				dds.dwCaps3 = 0;
				dds.dwCaps4 = 0;
				dds.dwReserved2 = 0;


				std::ofstream oFile(fileName, std::ofstream::binary);
				oFile.write((char*)&dds, sizeof(dds_header));

				oFile.write(dxtBuff.get(), textureDataSize);



				//printf("===================\n");
			}
			std::cout << "Finished." << std::endl;
		}
	}
	if (mode == MODE_CREATE)
	{
		if (!std::filesystem::exists(argv[argc - 1]))
		{
			std::cout << "ERROR: Folder does not exist: " << argv[argc - 1] << "!" << std::endl;
			return 1;
		}


		std::vector<fr3_texture> vTextures;
		std::vector<std::string> vNames;
		// write 


		std::string output = argv[argc - 1];
		output += ".dxt";

		if (!o_param.empty()) output = o_param;

		std::ofstream oFile(output, std::ofstream::binary);



		std::filesystem::current_path(argv[argc - 1]);


		if (t_param.empty())
		{
			std::cout << "ERROR: Texture data file not specified!" << std::endl;
			return 1;
		}

		if (l_param.empty())
		{
			std::cout << "ERROR: List file not specified!" << std::endl;
			return 1;
		}



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
			char filename[256];
			if (sscanf(szLine, "%s", &name) == 1)
			{

				int unk, unk_int, flags, flags2;
				float unk_float;

				sscanf(szLine, "%s %s %d %d %f %x %x ", &name, &filename, &unk, &unk_int, &unk_float, &flags, &flags2);

				fr3_texture tex;
				
				sprintf(tex.name, name);
				tex.unk = unk;
				tex.unk_int = unk_int;
				tex.unk_float = unk_float;
				tex.flags = flags;
				tex.flags2 = flags2;

				vTextures.push_back(tex);
				std::string str(filename, strlen(filename));
				vNames.push_back(str);

			}
		}


		std::ifstream pTextureData(t_param, std::ifstream::binary);


		if (!pTextureData)
		{
			std::cout << "ERROR: Failed to open texture data file: " << t_param.c_str() << "!" << std::endl;
			return 1;
		}


		int textureSize = std::filesystem::file_size(t_param);
		std::unique_ptr<char[]> textureBuff = std::make_unique<char[]>(textureSize);
		pTextureData.read(textureBuff.get(), textureSize);
		oFile.write(textureBuff.get(), textureSize);

		for (int i = 0; i < vTextures.size(); i++)
		{

			std::cout << "Processing: " << vNames[i].c_str() << std::endl;

			int ddsSize = std::filesystem::file_size(vNames[i]);

			std::ifstream pDDS(vNames[i], std::ifstream::binary);

			dds_header dds;
			pDDS.read((char*)&dds, sizeof(dds_header));

			ddsSize -= sizeof(dds_header);

			int ddsType = 1;

			if (dds.ddspf.dwFourCC[3] == '3') ddsType = 3;
			else if (dds.ddspf.dwFourCC[3] == '5') ddsType = 5;
			vTextures[i].dxtType = ddsType;
			vTextures[i].x = dds.dwWidth;
			vTextures[i].y = dds.dwHeight;

			// write texture header
			
			oFile.write((char*)&vTextures[i].flags, sizeof(int));

			int nameLen = strlen(vTextures[i].name);
			nameLen += 1;
			oFile.write((char*)&nameLen, sizeof(int));

			oFile.write((char*)&vTextures[i].name[0], nameLen);
			oFile.write((char*)&vTextures[i].dxtType, sizeof(int));
			oFile.write((char*)&vTextures[i].unk, sizeof(int));
			oFile.write((char*)&vTextures[i].unk_float, sizeof(float));
			oFile.write((char*)&vTextures[i].x, sizeof(int));
			oFile.write((char*)&vTextures[i].y, sizeof(int));
			oFile.write((char*)&vTextures[i].unk_int, sizeof(int));
			oFile.write((char*)&vTextures[i].flags2, sizeof(int));

			std::unique_ptr<char[]> ddsData = std::make_unique<char[]>(ddsSize);
			pDDS.read(ddsData.get(), ddsSize);
			oFile.write(ddsData.get(), ddsSize);
			
		}
		std::cout << "Finished." << std::endl;

	}
	if (mode == MODE_COMPRESS)
	{
		std::ifstream pFile(argv[argc - 1], std::ifstream::binary);

		if (!pFile)
		{
			std::cout << "ERROR: Could not open: " << argv[argc - 1] << "!" << std::endl;
			return 1;
		}

		if (pFile)
		{
			int size = std::filesystem::file_size(argv[argc - 1]);

			std::unique_ptr<char[]> dataBuff = std::make_unique<char[]>(size);
			pFile.read(dataBuff.get(), size);




			// create compression buff (+20%)
			unsigned long compSize = size * 1.2 + 12;
			std::unique_ptr<char[]> compBuff = std::make_unique<char[]>(compSize);


			int zlib_output = compress((Bytef*)compBuff.get(), &compSize, (Bytef*)dataBuff.get(), size);

			if (zlib_output == Z_MEM_ERROR) {
				std::cout << "ERROR: ZLIB: Out of memory!" << std::endl;
				return 1;
			}

			std::string output = "z_";
			output += argv[argc - 1];

			std::ofstream oFile(output, std::ofstream::binary);
			oFile.write((char*)&size, sizeof(int));

			oFile.write(compBuff.get(), compSize);

			std::cout << "Compressed file " << argv[argc - 1] << " as " << output.c_str() << "!" << std::endl;
		}

	}

}
