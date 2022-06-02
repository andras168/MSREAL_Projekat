#include <stdio.h>
#include <string.h>
#include <stdlib.h>


int main() {

	char st[100];
	
	printf("Enter the string: ");
	scanf("%s", st);
	printf("The entered string is %s.", st);

	int i=0,j=0;
	char copy[20]; //za smestanje cifara
	char *ch; //pointer za prelazak kroz string

	int arr[50]; //niz za popunjavanje matrice

	ch=st;

	while(*ch!='\0') {
		printf("%s\t",ch);

		if(*ch==';') 
		{i++;}
		else if(*ch==',')
		{j++;}
		else {
			strncat(copy, ch, 1);
			arr[j+i]=atoi(ch);
		}

		ch++;
	}

	int mat[i][j];

	printf("Number string: \n%s\n",copy);
	printf("\nNumber of rows: %d",i);
	printf("\nNumber of coloumns: %d\n",(j+i)/i);

	for(int c=0;c<j+i;c++) {
		printf("%d\t",arr[c]);
	}

	printf("\n\n Matrix  \n\n");
	int c=0;
	for(int a=0;a<i;a++) {
		for(int b=0;b<((j+i)/i);b++){
			mat[a][b]=arr[c];
			c++;
		} 
	}


	for(int a=0;a<i;a++) {
		for(int b=0;b<((j+i)/i);b++){
			printf("%d\t",mat[a][b]);

		}
		printf("\n\n");
	}


	return 0;
}
