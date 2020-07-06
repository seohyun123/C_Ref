#include	<stdio.h>
typedef unsigned char	byte;
FILE	*fp;

int	load_data(char *fname, void *buf, char type, int col, int row)
{
	if ((fp = fopen(fname, "rb"))==NULL) {
		printf("Unable to open file %s\n", fname);
		return(0);
	}

	switch( type )
	{
		case	'i' : fread((char*)buf, sizeof(int)*col, row, fp);
			      break;
		case	's' : fread((char*)buf, sizeof(short)*col, row, fp);
			      break;
		case	'b' : fread((char*)buf, col, row, fp);
			      break;
		default	    : fread((char*)buf, sizeof(float)*col, row, fp);
	}

	fclose(fp);
	return(1);
}

void	save_data(char *fname, void *buf, char type, int col, int row)
{
	fp = fopen(fname, "wb");
	switch( type )
	{
		case	'i' : fwrite((char*)buf, sizeof(int)*col, row, fp);
			      break;
		case	's' : fwrite((char*)buf, sizeof(short)*col, row, fp);
			      break;
		case	'b' : fwrite((char*)buf, col, row, fp);
			      break;
		default	    : fwrite((char*)buf, sizeof(float)*col, row, fp);
	}
	fclose(fp);
}

