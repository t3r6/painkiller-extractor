/*************************************************************************************/
/*	PainKiller Resource Extractor
/*	(c) Andrew Frolov aka FAL
/*	http:\\falinc.narod.ru
/*	falinc@ukr.net
/*	18.06.2004
/*************************************************************************************/

#include <direct.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <conio.h>
#include <ctype.h>
#include <assert.h>

extern "C" {
int puff(unsigned char *dest,           /* pointer to destination pointer */
         unsigned long *destlen,        /* amount of output space */
         unsigned char *source,         /* pointer to source data pointer */
         unsigned long *sourcelen);     /* amount of input available */
};

int main(int argc, char **argv)
{
	if(argc < 2) 
	{
		printf("PainKiller Resource Extractor\n"
				 "(c) Andrew Frolov aka FAL\n"
				 "http:\\falinc.narod.ru\n"
				 "falinc@ukr.net\n"
				 "18.06.2004\n\n"
				 "Usage: PainKillerExtractor <filename.pak>\n");
		return -1;
	}
	FILE* inDir = fopen(argv[1], "rb");
	FILE* inFiles = fopen(argv[1], "rb");
	if(!inDir || !inFiles)
	{
		printf("Cannot find file: %s\n", argv[1]);
		return -1;
	}
	
	bool isCompressed = !!fgetc(inDir);
	unsigned DirOffset = 0;
	fread(&DirOffset, 1, 4, inDir);
	fseek(inDir, DirOffset, SEEK_SET);
	unsigned FilesNumber = 0;
	fread(&FilesNumber, 1, 4, inDir);
	printf("Files in package: %u\n", FilesNumber);

	char CurDir[_MAX_PATH];
	getcwd(CurDir, _MAX_PATH);

	char OutDir[_MAX_PATH];
	sprintf(OutDir, "!%s", argv[1]);
	mkdir(OutDir);

	for(unsigned i = 0; i < FilesNumber; i++)
	{
		unsigned Len = 0;
		fread(&Len, 1, 4, inDir);
		char Name[_MAX_PATH];
		fread(Name, Len, 1, inDir);
		for(unsigned n = 0; n < Len; n++)
			Name[n] ^= (n + Len)*2 + (Len%5) + i;
		Name[Len] = 0;
		printf("%s\n", Name);

		unsigned FileOffset = 0;
		fread(&FileOffset, 1, 4, inDir);
		unsigned long OutLen = 0;
		fread(&OutLen, 1, 4, inDir);
		unsigned long InLen = 0;
		fread(&InLen, 1, 4, inDir);

		fseek(inFiles, FileOffset, SEEK_SET);
		
		if(isCompressed)
		{
			if(!(fgetc(inFiles) == 0x78 && fgetc(inFiles) == 0x01))
				printf("Unknown record!\n");
			else
			{
				unsigned char* InData = new unsigned char[InLen];
				unsigned char* OutData = new unsigned char[OutLen];
				fread(InData, 1, InLen, inFiles);
			
				unsigned char* WriteData = OutData;
				unsigned long WriteLen = OutLen;

				int ret = 0;
				if(InLen)
				{
					ret = puff(OutData, &OutLen, InData, &InLen);
					if(ret)
					{
						printf("Failed with return code %d\n", ret);
						WriteData = InData;
						WriteLen = InLen;
					}
				}
				if(ret == 0)
				{
					printf("Succeeded uncompressing %lu bytes\n", OutLen);
					WriteData = OutData;
					WriteLen = OutLen;
				}

				chdir(OutDir);
				char* dir = Name;
				char* p = strchr(dir, '/');
				while(p)
				{
					*p++ = 0;
					mkdir(dir);
					chdir(dir);
					dir = p;
					p = strchr(dir, '/');
				}

				char* FileName = dir;
				if(FileName && *FileName)
				{
					FILE* out = fopen(FileName, "wb");
					if(out)
					{
						fwrite(WriteData, 1, WriteLen, out);
						printf("%s saved\n", FileName);
						fclose(out);
					}
				}
				chdir(CurDir);
				delete[] InData;
				delete[] OutData;
			}
		}
		else
		{
			unsigned char* InData = new unsigned char[InLen];
			fread(InData, 1, InLen, inFiles);

			chdir(OutDir);
			char* dir = Name;
			char* p = strchr(dir, '/');
			while(p)
			{
				*p++ = 0;
				mkdir(dir);
				chdir(dir);
				dir = p;
				p = strchr(dir, '/');
			}

			char* FileName = dir;
			if(FileName && *FileName)
			{
				FILE* out = fopen(FileName, "wb");
				if(out)
				{
					fwrite(InData, 1, InLen, out);
					printf("%s saved\n", FileName);
					fclose(out);
				}
			}
			chdir(CurDir);
			delete[] InData;
		}
	}

	fclose(inDir);
	fclose(inFiles);
	return 0;
}
