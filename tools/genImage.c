#include <stdio.h>

int main(int argc, char *argv[])
{
	FILE *f_out;
	unsigned int width, height;
	int i,j;
	unsigned char r,g,b;

	if(argc != 4)
	{
		printf("Usage: genImage <output-file> <width> <height>\n");
		exit(1);
	}

	f_out = fopen(argv[1],"w+");
	if(f_out == NULL)
	{
		printf("File %s could not be opened for writing\n",argv[1]);
		exit(2);
	}

	width = atoi(argv[2]);
	height = atoi(argv[3]);

	fprintf(f_out,"P6 %d %d 255\n",width,height);
	for(i=0; i<height; i++)
	{
		for(j=0; j<width; j++)
		{
			r=i;
			g=i; /*255-i*/;
			b = i/*((i+j)>255) ? 255 - (i+j) : (i+j)*/;
			fwrite(&r,sizeof(unsigned char),1,f_out);
			fwrite(&g,sizeof(unsigned char),1,f_out);
			fwrite(&b,sizeof(unsigned char),1,f_out);
		}
	}
	fclose(f_out);
	return 0;
}
