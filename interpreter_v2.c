/*autor: Tiago Casagrande de Paula*/
/*The program accepts file names as arguments. The content of all files will be concatenated and treated as just one code.*/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>

#define letter(x) ((x >= 'a' && x <= 'z') || (x >= 'A' && x <= 'Z'))
#define digit(x) (x >= '0' && x <= '9')
#define HashM 0x1000

char stre(char *S, char *S0)
{
	for(int s=0; !(S[s]-S0[s]); s++)
		if(!(S[s]||S0[s])) return 1;
	return 0;
}

int main(int argc, char** argv)
{
	char *T, *I;
	int fd, *R, t, i, r, f, n, *M, m, *A, *L, l, bs, *H, h;
	struct stat b;
	void *C[]={&&push, &&deref, &&cat, &&ltz, &&neg, &&inc, &&dec, &&plus, &&opp, &&mult, &&div, &&mod, &&print, &&halt};

	for(t=0, n=1; n<argc; stat(argv[n++], &b), t+=b.st_size);
	T=(char*)malloc(sizeof(char)*t); //holds and concatenates the texts of all the input files
	for(t=0, n=1; n<argc; n++)
	{
		fd=open(argv[n], S_IREAD); stat(argv[n], &b);
		t+=read(fd, (void*)T+t, b.st_size);
		close(fd);
	}
	T[t]=0;

	I=(char*)malloc(sizeof(char)*t); //table of identifiers
	R=(int*)malloc(sizeof(int)*t); //references for the table of identifiers
	A=(int*)malloc(sizeof(int)*t); //holds the addresses of declared labels
	L=(int*)malloc(sizeof(int)*t); //hols all positions where labels appears
	M=(int*)malloc(sizeof(int)*t*2); //memory that holds the machine code and where it executes all its instructions
	H=(int*)malloc(sizeof(int)*HashM);

	for(i=0; i<t; A[i++]=-1);
	for(h=0; h<HashM; H[h++]=-1);
	m=0; t=0; i=0; r=0; l=0;
	base:
		if(!T[t]) goto end;
		if(letter(T[t])) {goto id;}
		if(digit(T[t])) {goto num;}
		if(T[t]=='@') {M[m++]=1; t++; goto base;}
		if(T[t]=='?') {M[m++]=2; t++; goto base;}
		if(T[t]=='<') {M[m++]=3; t++; goto base;}
		if(T[t]=='!') {M[m++]=4; t++; goto base;}
		if(T[t]=='[') {M[m++]=5; t++; goto base;}
		if(T[t]==']') {M[m++]=6; t++; goto base;}
		if(T[t]=='+') {M[m++]=7; t++; goto base;}
		if(T[t]=='-') {M[m++]=8; t++; goto base;}
		if(T[t]=='*') {M[m++]=9; t++; goto base;}
		if(T[t]=='/') {M[m++]=10; t++; goto base;}
		if(T[t]=='%') {M[m++]=11; t++; goto base;}
		if(T[t]=='|') {M[m++]=12; t++; goto base;}
		if(T[t]=='.') {M[m++]=13; t++; goto base;}
		if(T[t]=='(') {while(T[t++]!=')') if(!T[t]) {printf("Warning: comment reaches EOF."); goto end;} goto base;}
		t++;
		goto base;

	id:
		h=HashM/7; R[r]=i;
	id_:
		if(!letter(T[t]))
		{
			I[i++]=0;
			hash:
				if(H[h]==-1) {H[h]=r++; goto setlbl;}
				if(stre(I+R[r], I+R[H[h]])) {i=R[r]; goto setlbl;}
				h=(h+1)%HashM; goto hash;
			setlbl:
				if(T[t]==':') {A[H[h]]=m; t++; goto base;} // : defines the current label as the current address
				if(T[t]!=';') M[m++]=0; // ; permits a value to be written to the code without the stack up command
				L[l++]=m;
				M[m++]=H[h];
				goto base;
		}
		h=(3*h+5*T[t])%HashM;
		I[i++]=T[t++];
		goto id_;

	num:
		n=0; bs=10;
	num_:
		if(!digit(T[t]))
		{
			if(T[t]=='#') {m+=n; t++; goto base;} // # reserves memory. # is not a command
			if(T[t]=='b') {bs=2; t++; goto numa;} // 0b in the start of the number makes base 2
			if(T[t]=='-') {n*=-1; t++;} // - in the end of the number makes n negative
			if(T[t]!=';') M[m++]=0; // ; allows a value to be written in the code without the stack up command
			M[m++]=n;
			goto base;
		}
		numa:
			n=n*bs+T[t]-'0';
			t++;
		goto num_;

	end:

	for(n=0; n<l; (A[M[L[n]]]+1)?(0):(printf("Warning: label %s not declared.\n", I+R[M[L[n]]])), M[L[n]]=A[M[L[n]]], n++); //trades the identities on the labels by their actual addreses

	free(T); free(I); free(R); free(A); free(L); free(H);

	fetch: for(n=m, printf("\n"); n<M[1]; printf("%d ", M[n++]));
		goto *C[M[M[0]++]];

	push: M[M[1]++]=M[M[0]++]; goto fetch; //push
	deref: M[M[1]-1]=M[M[M[1]-1]]; goto fetch; //dereferentiation
	cat: if(M[M[1]-1]) M[M[M[1]-2]]=M[M[1]-3]; M[1]-=3; goto fetch; //conditional attribution (referentiation)
	ltz: if(M[M[1]-1]<0) M[M[1]-1]=1; else M[M[1]-1]=0; goto fetch; //true if less than 0
	neg: M[M[1]-1]=!M[M[1]-1]; goto fetch; //not
	inc: M[M[M[1]-1]]++; M[1]--; goto fetch; //increment
	dec: M[M[M[1]-1]]--; M[1]--; goto fetch; //decrement
	plus: M[M[1]-2]+=M[M[1]-1]; M[1]--; goto fetch; //plus
	opp: M[M[1]-1]*=-1; goto fetch; //opposite
	mult: M[M[1]-2]*=M[M[1]-1]; M[1]--; goto fetch; //multiply
	div: M[M[1]-2]/=M[M[1]-1]; M[1]--; goto fetch; //quotiente integer division
	mod: M[M[1]-2]%=M[M[1]-1]; M[1]--; goto fetch; //rest integer division
	print: f=M[M[1]-2]; n=f+M[M[1]-1]; M[1]-=2; for(printf("\n"); f<n; printf("%d ", M[f++])); printf("\n"); goto fetch; //label n | prints n numbers starting on label
	halt:

	free(M);

	return 0;
}

