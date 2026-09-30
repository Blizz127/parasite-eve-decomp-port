/* bandcorr A.wav tA B.wav tB dur win lo hi [shift]
   Per window of `win` s: Pearson correlation of log-magnitude spectrogram patches (bins lo..hi Hz)
   of A (starting tA) and B (starting tB), best over lag +-0.3 s; plus band energy dB of each.
   `shift` (s) is added to B's window start (baseline with a wrong alignment). */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <complex.h>
#define N 4096
#define HOP 1024
static void fft(double complex*x,int n){for(int i=1,j=0;i<n;i++){int b=n>>1;for(;j&b;b>>=1)j^=b;j^=b;if(i<j){double complex t=x[i];x[i]=x[j];x[j]=t;}}
 for(int len=2;len<=n;len<<=1){double complex w=cexp(-2*M_PI*I/len);for(int i=0;i<n;i+=len){double complex u=1;for(int j=0;j<len/2;j++){double complex a=x[i+j],b=x[i+j+len/2]*u;x[i+j]=a+b;x[i+j+len/2]=a-b;u*=w;}}}}
typedef struct{int nf,nb;float*s;double*e;}Spec;
static Spec spec(const char*fn,double t0,double dur,double lo,double hi){FILE*f=fopen(fn,"rb");uint8_t h[44];if(fread(h,1,44,f)!=44)exit(1);int sr=h[24]|h[25]<<8|h[26]<<16,ch=h[22];
 if(sr!=44100){fprintf(stderr,"need 44100\n");exit(1);}long n=(long)(dur*sr)+N;fseek(f,44+(long)(t0*sr)*2*ch,SEEK_SET);int16_t*a=calloc(n*ch,2);n=fread(a,2*ch,n,f);
 int k0=(int)(lo*N/sr),k1=(int)(hi*N/sr);Spec S;S.nb=k1-k0;S.nf=(int)((n-N)/HOP);S.s=malloc(sizeof(float)*S.nf*S.nb);S.e=malloc(sizeof(double)*S.nf);
 for(int fr=0;fr<S.nf;fr++){double complex x[N];for(int i=0;i<N;i++){double m=0;for(int c=0;c<ch;c++)m+=a[(fr*HOP+i)*ch+c];x[i]=m/ch*(0.5-0.5*cos(2*M_PI*i/N));}fft(x,N);
  double e=0;for(int k=k0;k<k1;k++){double p=creal(x[k]*conj(x[k]));e+=p;S.s[fr*S.nb+k-k0]=(float)log10(p+1e3);}S.e[fr]=e;}
 fclose(f);free(a);return S;}
int main(int c,char**v){double tA=atof(v[2]),tB=atof(v[4]),dur=atof(v[5]),win=atof(v[6]),lo=atof(v[7]),hi=atof(v[8]),sh=c>9?atof(v[9]):0;
 Spec A=spec(v[1],tA,dur,lo,hi),B=spec(v[3],tB+sh-0.5,dur+1.0,lo,hi);double fps=44100.0/HOP;int W=(int)(win*fps),L=(int)(0.3*fps),off=(int)(0.5*fps);
 printf("# t(s)  corr  lag(s)  EA(dB) EB(dB)\n");
 for(int w0=0;w0+W<=A.nf;w0+=W){double best=-2;int bl=0;
  for(int lag=-L;lag<=L;lag++){int b0=w0+off+lag;if(b0<0||b0+W>B.nf)continue;double sa=0,sb=0,saa=0,sbb=0,sab=0;long m=0;
   for(int i=0;i<W;i++)for(int k=0;k<A.nb;k++){double x=A.s[(w0+i)*A.nb+k],y=B.s[(b0+i)*B.nb+k];sa+=x;sb+=y;saa+=x*x;sbb+=y*y;sab+=x*y;m++;}
   double r=(sab-sa*sb/m)/sqrt((saa-sa*sa/m)*(sbb-sb*sb/m)+1e-12);if(r>best){best=r;bl=lag;}}
  double ea=0,eb=0;for(int i=0;i<W;i++){ea+=A.e[w0+i];eb+=B.e[w0+off+bl+i];}
  printf("%6.1f %5.2f %6.2f %6.1f %6.1f\n",w0/fps,best,bl/fps,10*log10(ea/W+1),10*log10(eb/W+1));}
}
