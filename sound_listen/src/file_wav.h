#ifndef FILE_WAV_H_
#define FILE_WAV_H_

#define WAV_HEADER_SIZE_IN_BYTES 44

#if 0
/** wav format from https://fr.wikipedia.org/wiki/Waveform_Audio_File_Format
[Bloc de d�claration d'un fichier au format WAVE]
   FileTypeBlocID  (4 octets) : Constante «RIFF»  (0x52,0x49,0x46,0x46)
   FileSize        (4 octets) : Taille du fichier moins 8 octets
   FileFormatID    (4 octets) : Format = «WAVE»  (0x57,0x41,0x56,0x45)

[Bloc d�crivant le format audio]
   FormatBlocID    (4 octets) : Identifiant «fmt »  (0x66,0x6D, 0x74,0x20)
   BlocSize        (4 octets) : Nombre d'octets du bloc - 16  (0x10)

   AudioFormat     (2 octets) : Format du stockage dans le fichier (1: PCM, ...)
   NbrCanaux       (2 octets) : Nombre de canaux (de 1 à 6, cf. ci-dessous)
   Frequence       (4 octets) : Fr�quence d'�chantillonnage (en hertz) [Valeurs standardis�es : 11 025, 22 050, 44 100 et éventuellement 48 000 et 96 000]
   BytePerSec      (4 octets) : Nombre d'octets à lire par seconde (c.-à-d., Frequence * BytePerBloc).
   BytePerBloc     (2 octets) : Nombre d'octets par bloc d'�chantillonnage (c.-à-d., tous canaux confondus : NbrCanaux * BitsPerSample/8).
   BitsPerSample   (2 octets) : Nombre de bits utilisés pour le codage de chaque échantillon (8, 16, 24)

[Bloc des donn�es]
   DataBlocID      (4 octets) : Constante «data»  (0x64,0x61,0x74,0x61)
   DataSize        (4 octets) : Nombre d'octets des donn�es (c.-à-d. "Data[]", c.-à-d. taille_du_fichier - taille_de_l'entête  (qui fait 44 octets normalement).
   DATAS[] : [Octets du Sample 1 du Canal 1] [Octets du Sample 1 du Canal 2] [Octets du Sample 2 du Canal 1] [Octets du Sample 2 du Canal 2]

   * Les Canaux :
      1 pour mono,
      2 pour stereo
      3 pour gauche, droit et centre
      4 pour face gauche, face droit, arrière gauche, arrière droit
      5 pour gauche, centre, droit, surround (ambiant)
      6 pour centre gauche, gauche, centre, centre droit, droit, surround (ambiant)

NOTES IMPORTANTES :  Les octets des mots sont stockés sous la forme  (c.-à-d., en "little endian")
[87654321][16..9][24..17] [8..1][16..9][24..17] [...
*/
typedef struct
{
	struct
	{
		CHAR acFileTypeBlocID[4]; //0x00
		UI32 u32FileSize;         //0x04
		CHAR acFileFormatID[4];   //0x08
	}stDeclarationArea;
	struct
	{
		CHAR acFormatBlocID[4];   //0x0C
		UI32 u32BlocSize;         //0x10
		UI16 u16AudioFormat;      //0x14
		UI16 u16NbrOfChannel;     //0x16
		UI32 u32Frequency;        //0x18
		UI32 u32BytePerSec;       //0x1C
		UI16 u16BytePerBloc;      //0x20
		UI16 u16BitsPerSample;    //0x22
	}stAudioFormat;
	struct
	{
		CHAR acDataBlocID[4];     //0x24
		UI32 u32DataSize;         //0x28
	}stDataBlock;
}WAV_stData;

typedef union
{
	WAV_stData xData;
	CHAR       cBuffer[sizeof(WAV_stData)];
}WAV_stHeaderFile;
#endif

typedef struct
{
	UI08 u8Buffer[WAV_HEADER_SIZE_IN_BYTES];
}WAV_stHeaderFile;

VOID FW_CreateHeaderFile(WAV_stHeaderFile* pHeader, UI32 u32NbSamples);
SI32 FW_Initialize (WAV_stHeaderFile* pHeader,CHAR pcWAVSourceFileName[]);
UI32 FW_ui32GetNbSample(WAV_stHeaderFile* pHeader);
VOID FW_vGetSignal(SI16* s16ReadSignal, UI32 u32NbElement);
VOID FW_Release(VOID);

#endif /* FILE_WAV_H_ */
