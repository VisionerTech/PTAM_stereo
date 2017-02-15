//**********************
// �e�N�X�`�����X�g�Ǘ�
//**********************

#include	<stdio.h>
#include	<malloc.h>
#include	<string.h>
#include	<gl/glut.h>
#include	"TextureList.h"

cTextureList	g_clsTextureList;

//================
// �R���X�g���N�^
//================
cTextureList::cTextureList( void ) : m_pTextureList( NULL )
{
}

//==============
// �f�X�g���N�^
//==============
cTextureList::~cTextureList( void )
{
	TextureData	*pTemp = m_pTextureList,
				*pNextTemp;

	while( pTemp )
	{
		pNextTemp = pTemp->pNext;

		glDeleteTextures( 1, &pTemp->uiTexID );
		delete pTemp;

		pTemp = pNextTemp;
	}

	m_pTextureList = NULL;
}

//====================
// �e�N�X�`��ID���擾
//====================
unsigned int cTextureList::getTexture( const char *szFileName )
{
	unsigned int	uiTexID = 0xFFFFFFFF;

	// �܂��͂��łɓǂݍ��܂�Ă��邩�ǂ�������
	if( findTexture( szFileName, &uiTexID ) )
	{
		return uiTexID;
	}

	// �Ȃ���΃t�@�C����ǂݍ���Ńe�N�X�`���쐬
	if( createTexture( szFileName, &uiTexID ) )
	{
		// �V�������X�g�m�[�h���쐬���擪�֐ڑ�
		TextureData	*pNew = new TextureData;

		strcpy( pNew->szFileName, szFileName );
		pNew->uiTexID = uiTexID;
		pNew->uiRefCount = 1;

		pNew->pNext = m_pTextureList;
		m_pTextureList = pNew;

		return uiTexID;
	}

	return 0xFFFFFFFF;	// �e�N�X�`���ǂݍ��݂��쐬���s
}

//----------------------------------
// �ǂݍ��ݍς݂̃e�N�X�`�����猟��
//----------------------------------
bool cTextureList::findTexture( const char *szFileName, unsigned int *puiTexID )
{
	TextureData	*pTemp = m_pTextureList;

	while( pTemp )
	{
		if( strcmp( pTemp->szFileName, szFileName ) == 0 )
		{
			// �ǂݍ��ݍς݂̃e�N�X�`���𔭌�
			*puiTexID = pTemp->uiTexID;
			pTemp->uiRefCount++;

			return true;
		}

		pTemp = pTemp->pNext;
	}

	return false;	// �����ł���
}

//--------------------------------------
// �t�@�C����ǂݍ���Ńe�N�X�`�����쐬
//--------------------------------------
bool cTextureList::createTexture( const char *szFileName, unsigned int *puiTexID )
{
	FILE	*pFile;
	long	fFileSize;
	unsigned char
			*pData;

	// TODO: fetch sphere mapping
	char fileName[256];
	const char *tmp_p = strrchr(szFileName, '*');
	int fileNameLength;
	if(!tmp_p) fileNameLength = strlen( szFileName );
	else fileNameLength = tmp_p - szFileName;
	strncpy(fileName, szFileName, fileNameLength);
	fileName[fileNameLength] = 0x0;

	pFile = fopen( fileName, "rb" );
	//pFile = fopen( szFileName, "rb" );
	if( !pFile )	return false;	// �t�@�C�����J���Ȃ�

	// �t�@�C���T�C�Y�擾
	fseek( pFile, 0, SEEK_END );
	fFileSize = ftell(pFile);

	// �������m��
	pData = (unsigned char *)malloc( (size_t)fFileSize );

	// �ǂݍ���
	fseek( pFile, 0, SEEK_SET );
	fread( pData, 1, (size_t)fFileSize, pFile );

	fclose( pFile );

	int		iLen = strlen( szFileName );
	bool	bRet = false;

	if( 	(szFileName[iLen - 3] == 'b' || szFileName[iLen - 3] == 'B') &&
			(szFileName[iLen - 2] == 'm' || szFileName[iLen - 2] == 'M') &&
			(szFileName[iLen - 1] == 'p' || szFileName[iLen - 1] == 'P')		)
	{
		bRet = createFromBMP( pData, puiTexID );
	}
	else if((szFileName[iLen - 3] == 's' || szFileName[iLen - 3] == 'S') &&
			(szFileName[iLen - 2] == 'p' || szFileName[iLen - 2] == 'P') &&
			(szFileName[iLen - 1] == 'h' || szFileName[iLen - 1] == 'H')		)
	{
		bRet = createFromBMP( pData, puiTexID );	// *.sph ��BMP
	}
	else if((szFileName[iLen - 3] == 't' || szFileName[iLen - 3] == 'T') &&
			(szFileName[iLen - 2] == 'g' || szFileName[iLen - 2] == 'G') &&
			(szFileName[iLen - 1] == 'a' || szFileName[iLen - 1] == 'A')		)
	{
		bRet = createFromTGA( pData, puiTexID );
	}

	free( pData );

	return bRet;
}

//-------------------------------
// BMP�t�@�C������e�N�X�`���쐬
//-------------------------------
bool cTextureList::createFromBMP( const unsigned char *pData, unsigned int *puiTexID )
{
#pragma pack( push, 1 )
	// BMP�t�@�C���w�b�_�\����
	struct BMPFileHeader
	{
		unsigned short	bfType;			// �t�@�C���^�C�v
		unsigned int	bfSize;			// �t�@�C���T�C�Y
		unsigned short	bfReserved1;
		unsigned short	bfReserved2;
		unsigned int	bfOffBits;		// �t�@�C���擪����摜�f�[�^�܂ł̃I�t�Z�b�g
	};

	// BMP���w�b�_�\����
	struct BMPInfoHeader
	{
		unsigned int	biSize;			// ���w�b�_�[�̃T�C�Y
		int				biWidth;		// ��
		int				biHeight;		// ����(���Ȃ�Ή������A���Ȃ�Ώォ�牺)
		unsigned short	biPlanes;		// �v���[����(���1)
		unsigned short	biBitCount;		// 1��f������̃r�b�g��
		unsigned int	biCompression;
		unsigned int	biSizeImage;
		int				biXPelsPerMeter;
		int				biYPelsPerMeter;
		unsigned int	biClrUsed;		// �p���b�g�̐F��
		unsigned int	biClrImportant;
	};

	// �p���b�g�f�[�^
	struct RGBQuad
	{
		unsigned char	rgbBlue;
		unsigned char	rgbGreen;
		unsigned char	rgbRed;
		unsigned char	rgbReserved;
	};
#pragma pack( pop )

	// BMP�t�@�C���w�b�_
	BMPFileHeader	*pBMPFileHeader = (BMPFileHeader *)pData;

	if( pBMPFileHeader->bfType != ('B' | ('M' << 8)) )
	{
		return false;	// �t�@�C���^�C�v���Ⴄ
	}

	// BMP���w�b�_
	BMPInfoHeader	*pBMPInfoHeader = (BMPInfoHeader *)(pData + sizeof(BMPFileHeader));

	if( pBMPInfoHeader->biBitCount == 1 || pBMPInfoHeader->biCompression != 0 )
	{
		return false;	// 1�r�b�g�J���[�ƈ��k�`���ɂ͖��Ή�
	}

	// �J���[�p���b�g
	RGBQuad			*pPalette = NULL;

	if( pBMPInfoHeader->biBitCount < 24 )
	{
		pPalette = (RGBQuad *)(pData + sizeof(BMPFileHeader) + sizeof(BMPInfoHeader));
	}

	// �摜�f�[�^�̐擪��
	pData += pBMPFileHeader->bfOffBits;

	// �摜�f�[�^��1���C���̃o�C�g��
	unsigned int	uiLineByte = ((pBMPInfoHeader->biWidth * pBMPInfoHeader->biBitCount + 0x1F) & (~0x1F)) / 8;

	// �e�N�X�`���C���[�W�̍쐬
	unsigned char	*pTexelData = (unsigned char *)malloc( pBMPInfoHeader->biWidth * pBMPInfoHeader->biHeight * 4 ),
					*pTexelDataTemp = pTexelData;

	if( pBMPInfoHeader->biBitCount == 4 )
	{
		// 4Bit�J���[
		for( int h = pBMPInfoHeader->biHeight - 1 ; h >= 0 ; h-- )
		{
			const unsigned char *pLineTop = &pData[uiLineByte * h];

			for( int w = 0 ; w < (pBMPInfoHeader->biWidth >> 1) ; w++ )
			{
				*pTexelDataTemp = pPalette[(pLineTop[w] >> 4) & 0x0F].rgbRed;	pTexelDataTemp++;
				*pTexelDataTemp = pPalette[(pLineTop[w] >> 4) & 0x0F].rgbGreen;	pTexelDataTemp++;
				*pTexelDataTemp = pPalette[(pLineTop[w] >> 4) & 0x0F].rgbBlue;	pTexelDataTemp++;
				*pTexelDataTemp = 255;											pTexelDataTemp++;

				*pTexelDataTemp = pPalette[(pLineTop[w]     ) & 0x0F].rgbRed;	pTexelDataTemp++;
				*pTexelDataTemp = pPalette[(pLineTop[w]     ) & 0x0F].rgbGreen;	pTexelDataTemp++;
				*pTexelDataTemp = pPalette[(pLineTop[w]     ) & 0x0F].rgbBlue;	pTexelDataTemp++;
				*pTexelDataTemp = 255;											pTexelDataTemp++;
			}
		}
	}
	else if( pBMPInfoHeader->biBitCount == 8 )
	{
		// 8Bit�J���[
		for( int h = pBMPInfoHeader->biHeight - 1 ; h >= 0 ; h-- )
		{
			const unsigned char *pLineTop = &pData[uiLineByte * h];

			for( int w = 0 ; w < pBMPInfoHeader->biWidth ; w++ )
			{
				*pTexelDataTemp = pPalette[pLineTop[w]].rgbRed;		pTexelDataTemp++;
				*pTexelDataTemp = pPalette[pLineTop[w]].rgbGreen;	pTexelDataTemp++;
				*pTexelDataTemp = pPalette[pLineTop[w]].rgbBlue;	pTexelDataTemp++;
				*pTexelDataTemp = 255;								pTexelDataTemp++;
			}
		}
	}
	else if( pBMPInfoHeader->biBitCount == 24 )
	{
		// 24Bit�J���[
		for( int h = pBMPInfoHeader->biHeight - 1 ; h >= 0 ; h-- )
		{
			const unsigned char *pLineTop = &pData[uiLineByte * h];

			for( int w = 0 ; w < pBMPInfoHeader->biWidth ; w++ )
			{
				*pTexelDataTemp = pLineTop[w * 3 + 2];	pTexelDataTemp++;
				*pTexelDataTemp = pLineTop[w * 3 + 1];	pTexelDataTemp++;
				*pTexelDataTemp = pLineTop[w * 3    ];	pTexelDataTemp++;
				*pTexelDataTemp = 255;					pTexelDataTemp++;
			}
		}
	}
	else if( pBMPInfoHeader->biBitCount == 32 )
	{
		// 32Bit�J���[
		for( int h = pBMPInfoHeader->biHeight - 1 ; h >= 0 ; h-- )
		{
			const unsigned char *pLineTop = &pData[uiLineByte * h];

			for( int w = 0 ; w < pBMPInfoHeader->biWidth ; w++ )
			{
				*pTexelDataTemp = pLineTop[w * 4 + 2];	pTexelDataTemp++;
				*pTexelDataTemp = pLineTop[w * 4 + 1];	pTexelDataTemp++;
				*pTexelDataTemp = pLineTop[w * 4    ];	pTexelDataTemp++;
				*pTexelDataTemp = 255;					pTexelDataTemp++;
			}
		}
	}

	// �e�N�X�`���̍쐬
	glGenTextures( 1, puiTexID );

	glBindTexture( GL_TEXTURE_2D, *puiTexID );
	glPixelStorei( GL_UNPACK_ALIGNMENT, 4 );

	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );

	glTexImage2D(	GL_TEXTURE_2D, 0, GL_RGBA, 
					pBMPInfoHeader->biWidth, pBMPInfoHeader->biHeight,
					0, GL_RGBA, GL_UNSIGNED_BYTE,
					pTexelData );

	float	fPrioritie = 1.0f;
	glPrioritizeTextures( 1, puiTexID, &fPrioritie );

	free( pTexelData );

	return true;
}

//-------------------------------
// TGA�t�@�C������e�N�X�`���쐬
//-------------------------------
bool cTextureList::createFromTGA( const unsigned char *pData, unsigned int *puiTexID )
{
#pragma pack( push, 1 )
	struct TGAFileHeader
	{
		unsigned char	tfIdFieldLength;
		unsigned char	tfColorMapType;
		unsigned char	tfImageType;
		unsigned short	tfColorMapIndex;
		unsigned short	tfColorMapLength;
		unsigned char	tfColorMapSize;
		unsigned short	tfImageOriginX;
		unsigned short	tfImageOriginY;
		unsigned short	tfImageWidth;
		unsigned short	tfImageHeight;
        unsigned char	tfBitPerPixel;
        unsigned char	tfDiscripter;
	};
#pragma pack( pop )

	TGAFileHeader	*pTgaFileHeader = (TGAFileHeader *)pData;

/*
0	�C���[�W�Ȃ�
1	�C���f�b�N�X�J���[�i256�F�j
2	�t���J���[
3	����
9	�C���f�b�N�X�J���[�BRLE���k
A	�t���J���[�BRLE���k
B	�����BRLE���k
*/
	if( pTgaFileHeader->tfImageType != 0x02 && pTgaFileHeader->tfImageType != 0x0A )
	{
		// ��Ή��t�H�[�}�b�g
		return false;
	}

	pData += sizeof( TGAFileHeader );

	unsigned char	*pTexelData = (unsigned char *)malloc( pTgaFileHeader->tfImageWidth * pTgaFileHeader->tfImageHeight * 4 ),
					*pTexelDataTemp = pTexelData;

	if( pTgaFileHeader->tfImageType == 0x02 && pTgaFileHeader->tfBitPerPixel == 24 )
	{
		// �񈳏k24Bit�J���[
		if( pTgaFileHeader->tfDiscripter & 0x20 )
		{
			// �ォ�牺��
			for( int h = 0 ; h < pTgaFileHeader->tfImageHeight ; h++ )
			{
				const unsigned char *pLineTop = &pData[(pTgaFileHeader->tfImageWidth * 3) * h];

				for( int w = 0 ; w < pTgaFileHeader->tfImageWidth ; w++ )
				{
					*pTexelDataTemp = pLineTop[w * 3 + 2];	pTexelDataTemp++;
					*pTexelDataTemp = pLineTop[w * 3 + 1];	pTexelDataTemp++;
					*pTexelDataTemp = pLineTop[w * 3    ];	pTexelDataTemp++;
					*pTexelDataTemp = 255;					pTexelDataTemp++;
				}
			}
		}
		else
		{
			// ��������
			for( int h = pTgaFileHeader->tfImageHeight - 1 ; h >= 0 ; h-- )
			{
				const unsigned char *pLineTop = &pData[(pTgaFileHeader->tfImageWidth * 3) * h];

				for( int w = 0 ; w < pTgaFileHeader->tfImageWidth ; w++ )
				{
					*pTexelDataTemp = pLineTop[w * 3 + 2];	pTexelDataTemp++;
					*pTexelDataTemp = pLineTop[w * 3 + 1];	pTexelDataTemp++;
					*pTexelDataTemp = pLineTop[w * 3    ];	pTexelDataTemp++;
					*pTexelDataTemp = 255;					pTexelDataTemp++;
				}
			}
		}
	}
	else if( pTgaFileHeader->tfImageType == 0x02 && pTgaFileHeader->tfBitPerPixel == 32 )
	{
		// �񈳏k32Bit�J���[
		if( pTgaFileHeader->tfDiscripter & 0x20 )
		{
			// �ォ�牺��
			for( int h = 0 ; h < pTgaFileHeader->tfImageHeight ; h++ )
			{
				const unsigned char *pLineTop = &pData[(pTgaFileHeader->tfImageWidth * 4) * h];

				for( int w = 0 ; w < pTgaFileHeader->tfImageWidth ; w++ )
				{
					*pTexelDataTemp = pLineTop[w * 4 + 2];	pTexelDataTemp++;
					*pTexelDataTemp = pLineTop[w * 4 + 1];	pTexelDataTemp++;
					*pTexelDataTemp = pLineTop[w * 4    ];	pTexelDataTemp++;
					*pTexelDataTemp = pLineTop[w * 4 + 3];	pTexelDataTemp++;
				}
			}
		}
		else
		{
			// ��������
			for( int h = pTgaFileHeader->tfImageHeight - 1 ; h >= 0 ; h-- )
			{
				const unsigned char *pLineTop = &pData[(pTgaFileHeader->tfImageWidth * 4) * h];

				for( int w = 0 ; w < pTgaFileHeader->tfImageWidth ; w++ )
				{
					*pTexelDataTemp = pLineTop[w * 4 + 2];	pTexelDataTemp++;
					*pTexelDataTemp = pLineTop[w * 4 + 1];	pTexelDataTemp++;
					*pTexelDataTemp = pLineTop[w * 4    ];	pTexelDataTemp++;
					*pTexelDataTemp = pLineTop[w * 4 + 3];	pTexelDataTemp++;
				}
			}
		}
	}
	else if( pTgaFileHeader->tfImageType == 0x0A )
	{
		// ���k24/32Bit�J���[
		if( pTgaFileHeader->tfDiscripter & 0x20 )
		{
			// �ォ�牺��
			short	nPosX = 0,
					nPosY = 0;

            while( nPosY < pTgaFileHeader->tfImageHeight )
            {
                bool	bCompress =	((*pData) & 0x80) == 0x80;
                short	nLength = ((*pData) & 0x7F) + 1;

				pData++;

                if( bCompress )
                {
                    for( short i = 0 ; i < nLength ; i++ )
                    {
						*pTexelDataTemp = pData[2];	pTexelDataTemp++;
						*pTexelDataTemp = pData[1];	pTexelDataTemp++;
						*pTexelDataTemp = pData[0];	pTexelDataTemp++;

						if( pTgaFileHeader->tfBitPerPixel == 32 )	*pTexelDataTemp = pData[3];
						else										*pTexelDataTemp = 255;
						pTexelDataTemp++;

						nPosX++;
						if( pTgaFileHeader->tfImageWidth <= nPosX )
						{
							nPosX = 0;
							nPosY++;
						}
                    }

					if( pTgaFileHeader->tfBitPerPixel == 32 )	pData += 4;
					else										pData += 3;
                }
                else
                {
                    for( short i = 0 ; i < nLength ; i++ )
                    {
						*pTexelDataTemp = pData[2];	pTexelDataTemp++;
						*pTexelDataTemp = pData[1];	pTexelDataTemp++;
						*pTexelDataTemp = pData[0];	pTexelDataTemp++;

						if( pTgaFileHeader->tfBitPerPixel == 32 )	*pTexelDataTemp = pData[3];
						else										*pTexelDataTemp = 255;
						pTexelDataTemp++;

						if( pTgaFileHeader->tfBitPerPixel == 32 )	pData += 4;
						else										pData += 3;

                        nPosX++;
                        if( pTgaFileHeader->tfImageWidth <= nPosX )
                        {
                            nPosX = 0;
                            nPosY++;
                        }
                    }
                }
            }
		}
		else
		{
			// ��������
			short	nPosX = 0,
					nPosY = pTgaFileHeader->tfImageHeight - 1;

            while( 0 <= nPosY )
            {
                bool	bCompress =	((*pData) & 0x80) == 0x80;
                short	nLength = ((*pData) & 0x7F) + 1;

				pData++;

                if( bCompress )
                {
                    for( short i = 0 ; i < nLength ; i++ )
                    {
						pTexelDataTemp = &pTexelData[(nPosX + nPosY * pTgaFileHeader->tfImageWidth) * 4];

						*pTexelDataTemp = pData[2];	pTexelDataTemp++;
						*pTexelDataTemp = pData[1];	pTexelDataTemp++;
						*pTexelDataTemp = pData[0];	pTexelDataTemp++;

						if( pTgaFileHeader->tfBitPerPixel == 32 )	*pTexelDataTemp = pData[3];
						else										*pTexelDataTemp = 255;

						nPosX++;
						if( pTgaFileHeader->tfImageWidth <= nPosX )
						{
							nPosX = 0;
							nPosY--;
						}
                    }

					if( pTgaFileHeader->tfBitPerPixel == 32 )	pData += 4;
					else										pData += 3;
                }
                else
                {
                    for( short i = 0 ; i < nLength ; i++ )
                    {
						pTexelDataTemp = &pTexelData[(nPosX + nPosY * pTgaFileHeader->tfImageWidth) * 4];

						*pTexelDataTemp = pData[2];	pTexelDataTemp++;
						*pTexelDataTemp = pData[1];	pTexelDataTemp++;
						*pTexelDataTemp = pData[0];	pTexelDataTemp++;

						if( pTgaFileHeader->tfBitPerPixel == 32 )	*pTexelDataTemp = pData[3];
						else										*pTexelDataTemp = 255;

						if( pTgaFileHeader->tfBitPerPixel == 32 )	pData += 4;
						else										pData += 3;

                        nPosX++;
                        if( pTgaFileHeader->tfImageWidth <= nPosX )
                        {
                            nPosX = 0;
                            nPosY--;
                        }
                    }
                }
            }
		}
	}

	// �e�N�X�`���̍쐬
	glGenTextures( 1, puiTexID );

	glBindTexture( GL_TEXTURE_2D, *puiTexID );
	glPixelStorei( GL_UNPACK_ALIGNMENT, 4 );

	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );

	glTexImage2D(	GL_TEXTURE_2D, 0, GL_RGBA, 
					pTgaFileHeader->tfImageWidth, pTgaFileHeader->tfImageHeight,
					0, GL_RGBA, GL_UNSIGNED_BYTE,
					pTexelData );

	float	fPrioritie = 1.0f;
	glPrioritizeTextures( 1, puiTexID, &fPrioritie );

	free( pTexelData );

	return true;
}

//==============
// �f�o�b�O�\��
//==============
void cTextureList::debugDraw( void )
{
	glDisable( GL_DEPTH_TEST );
	glDisable( GL_LIGHTING );

	glMatrixMode( GL_PROJECTION );
	glPushMatrix();
	glLoadIdentity();
	gluOrtho2D( 0.0f, (float)glutGet( GLUT_WINDOW_WIDTH ), 0.0f, (float)glutGet( GLUT_WINDOW_HEIGHT ) );

	glMatrixMode( GL_MODELVIEW );
	glPushMatrix();
	glLoadIdentity();

	glColor4f( 1.0f, 1.0f, 1.0f, 1.0f );

	glEnable( GL_TEXTURE_2D );

	TextureData	*pTemp = m_pTextureList;
	float		fPosX = 0.0f,
				fPosY = glutGet( GLUT_WINDOW_HEIGHT );

	#define		DISP_SIZE	64.0f

	while( pTemp )
	{
		glBindTexture( GL_TEXTURE_2D, pTemp->uiTexID );

		glBegin( GL_TRIANGLE_FAN );
			glTexCoord2f( 1.0f, 1.0f );	glVertex2f( fPosX,             fPosY - DISP_SIZE );
			glTexCoord2f( 1.0f, 0.0f );	glVertex2f( fPosX + DISP_SIZE, fPosY - DISP_SIZE );
			glTexCoord2f( 0.0f, 0.0f );	glVertex2f( fPosX + DISP_SIZE, fPosY             );
			glTexCoord2f( 0.0f, 1.0f );	glVertex2f( fPosX,             fPosY             );
		glEnd();

		fPosX += DISP_SIZE + 2.0f;
		if( fPosX >= glutGet( GLUT_WINDOW_WIDTH ) - DISP_SIZE )
		{
			fPosX  =             0.0f;
			fPosY -= DISP_SIZE + 2.0f;
		}

		pTemp = pTemp->pNext;
	}

	glMatrixMode( GL_PROJECTION );
	glPopMatrix();
	glMatrixMode( GL_MODELVIEW );
	glPopMatrix();

	glEnable( GL_DEPTH_TEST );
	glEnable( GL_LIGHTING );
}

//================
// �e�N�X�`�����
//================
void cTextureList::releaseTexture( unsigned int uiTexID )
{
	TextureData	*pTemp = m_pTextureList,
				*pPrevTemp = NULL;

	while( pTemp )
	{
		if( pTemp->uiTexID == uiTexID )
		{
			pTemp->uiRefCount--;
			if( pTemp->uiRefCount <= 0 )
			{
				glDeleteTextures( 1, &pTemp->uiTexID );

				if( pPrevTemp )	pPrevTemp->pNext = pTemp->pNext;
				else			m_pTextureList = pTemp->pNext;

				delete pTemp;
			}

			return;
		}

		pPrevTemp = pTemp;
		pTemp = pTemp->pNext;
	}
}
