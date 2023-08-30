#include "test.h"
#include <iostream>

#define UNROLL_X 4

#define Tr 7
#define Tc 7
#define Tm 8
#define Tn 8

#define H   14
#define W   14
#define ICH 16
#define OCH 16
#define K 3


//typedef ap_fixed<27,5> fix;
typedef ap_fixed<27,5> fix;


void load_input(const float* conv2d_input_1, int ti, int idx_n, int row, int idx_r, int col, int idx_c, int R, int C, int M, int N, fix input_1 [Tn][Tr+K-1][Tc+K-1]){

	for(int tii=0; tii<Tn; tii++){
		for(int trr=0; trr<Tr+K-1; trr++){
			for(int tcc=0; tcc<Tc+K-1; tcc++){
				#pragma HLS PIPELINE II=1

				int ph = trr +row- K/2;
				int pw = tcc +col- K/2;

				fix tmp;

				if(tii>=idx_n||trr>=idx_r+K-1||tcc>=idx_c+K-1)tmp=0;
				else if (ph < 0 || ph >= R || pw < 0 || pw >= C)tmp=0;
				else tmp=*(conv2d_input_1+((tii+ti)*R*C+ph*C+pw));

				input_1[tii][trr][tcc]=tmp;

			}
		}
	}

	return;
}

/*
void load_kernel(const float* conv2d_input_2, int to, int idx_m, int ti, int idx_n, int R, int C, int M, int N, fix kernel_1[Tm][Tn][K][K]){

	for(int too=0; too<Tm; too++){
		for(int tii=0; tii<Tn; tii++){
			for(int i =0; i<K; i++){
				for(int j =0; j<K; j++){
					#pragma HLS PIPELINE II=1
					fix tmp;
					if(too<idx_m&&tii<idx_n){
						tmp=*(conv2d_input_2+((too+to)*N*K*K+(tii+ti)*K*K+i*K+j));
						kernel_1[too][tii][i][j]=tmp;
					}
				}
			}
		}
	}

	return;
}
*/

void load_kernel(const float* conv2d_input_2, int to, int idx_m, int ti, int idx_n, int R, int C, int M, int N, fix kernel_1[Tm][Tn][K][K]){

	for(int too=0; too<Tm; too++){
		for(int idx=0; idx<Tn*K*K; idx++){
			//for(int i =0; i<K; i++){
				//for(int j =0; j<K; j++){
					#pragma HLS PIPELINE II=1
					fix tmp;

					int tii=idx/(K*K);
					int temp=idx%(K*K);
					int i=temp/K;
					int j=temp%K;
					//if(too<idx_m&&tii<idx_n){
						tmp=*(conv2d_input_2+((too+to)*N*K*K+idx));
						kernel_1[too][tii][i][j]=tmp;
					//}
				//}
			//}
		}
	}

	return;
}
/*
void load_output(float* conv2d_output, int to, int idx_m, int row, int idx_r, int col, int idx_c, int R, int C, int M, int N, fix output_1[Tm][Tr][Tc]){

	for(int too=0; too<Tm; too++){
		for(int trr=0; trr<Tr; trr++){
			for(int tcc=0; tcc<Tc; tcc++){
				#pragma HLS PIPELINE II=1
				fix tmp;
				if(too<idx_m&&trr<idx_r&&tcc<idx_c){
					tmp=*(conv2d_output+((too+to)*R*C+(trr+row)*C+tcc+col));
					output_1[too][trr][tcc]=tmp;
				}
			}
		}
	}

	return;
}
*/
/*
void clean_output(float* conv2d_output, int to, int idx_m, int row, int idx_r, int col, int idx_c, int R, int C, int M, int N, fix output_1[Tm][Tr][Tc]){

	for(int too=0; too<Tm; too++){
		for(int trr=0; trr<Tr; trr++){
			for(int tcc=0; tcc<Tc; tcc++){
				#pragma HLS PIPELINE II=1
				output_1[too][trr][tcc]=0;

			}
		}
	}

	return;
}
*/
void compute(int idx_r, int idx_c, int idx_m, int idx_n, fix output_1[Tm][Tr][Tc], fix kernel_1[Tm][Tn][K][K], fix input_1 [Tn][Tr+K-1][Tc+K-1], int ti){

	fix temp1[Tm][Tn];
	fix temp[Tm];

	for(int i =0; i<K; i++){
		for(int j =0; j<K; j++){

			for(int trr=0; trr<Tr; trr++){
				for(int tcc=0; tcc<Tc; tcc++){
					#pragma HLS PIPELINE II=1


					for(int too=0; too<Tm; too++){//unroll
						#pragma HLS UNROLL
						for(int tii=0; tii<Tn; tii++){//unroll
							#pragma HLS UNROLL
							if(tii<idx_n&&too<idx_m&&trr<idx_r&&tcc<idx_c){
								temp1[too][tii]=kernel_1[too][tii][i][j]*input_1[tii][trr+i][tcc+j];
							}
							else{
								temp1[too][tii]=0;
							}
						}
					}

					for(int too=0; too<Tm; too++){//unroll
						temp[too]=0;
						for(int tii=0; tii<Tn; tii++){//unroll
							#pragma HLS UNROLL
							temp[too]+=temp1[too][tii];

						}
						if(i==0&&j==0&&ti==0){
							temp[too]=temp[too];
						}
						else{
							temp[too]+=output_1[too][trr][tcc];
						}

					}
					for(int too=0; too<Tm; too++){//unroll
						#pragma HLS UNROLL
						if(too<idx_m&&trr<idx_r&&tcc<idx_c)output_1[too][trr][tcc]=temp[too];
					}

				}
			}


		}
	}

}

void store_output(float* conv2d_output, int to, int idx_m, int row, int idx_r, int col, int idx_c, int R, int C, int M, int N, fix output_1[Tm][Tr][Tc]){

	for(int too=0; too<Tm; too++){
			for(int trr=0; trr<Tr; trr++){
				for(int tcc=0; tcc<Tc; tcc++){
					#pragma HLS PIPELINE II=1
					fix tmp;
					//if(too>=idx_m||trr>=idx_r||tcc>=idx_c)
						//continue;

					*(conv2d_output+((too+to)*R*C+(trr+row)*C+tcc+col))=(float)output_1[too][trr][tcc];

				}
			}
		}

	return;
}


void test(const float* conv2d_input_1, const float* conv2d_input_2, int width, int height,
        int in_channels, int out_channels, int ksize, float* conv2d_output,int stride) {

#pragma HLS INTERFACE s_axilite port=width
#pragma HLS INTERFACE s_axilite port=height
#pragma HLS INTERFACE s_axilite port=in_channels
#pragma HLS INTERFACE s_axilite port=out_channels
#pragma HLS INTERFACE s_axilite port=ksize
#pragma HLS INTERFACE s_axilite port=stride
#pragma HLS INTERFACE m_axi depth=16*14*14 port=conv2d_input_1 offset=slave bundle=INPUT1 latency=10
#pragma HLS INTERFACE m_axi depth=16*16*3*3 port=conv2d_input_2 offset=slave bundle=INPUT2 latency=10
#pragma HLS INTERFACE m_axi depth=16*14*14 port=conv2d_output offset=slave bundle=OUTPUT latency=10
#pragma HLS INTERFACE s_axilite register port=return

#pragma HLS DATAFLOW

	int R=height;
	int C=width;
	int M=out_channels;
	int N=in_channels;

	fix input_1   [Tn][Tr+K-1][Tc+K-1];
	fix kernel_1  [Tm][Tn][K][K];
	fix output_1  [Tm][Tr][Tc];
	fix input_2   [Tn][Tr+K-1][Tc+K-1];
	fix kernel_2  [Tm][Tn][K][K];
	fix output_2  [Tm][Tr][Tc];


//#pragma HLS ARRAY_PARTITION variable=input_1 complete
//#pragma HLS ARRAY_PARTITION variable=output_1 complete
//#pragma HLS ARRAY_PARTITION variable=kernel_1 complete


	int i,j,k,l;


	int idx_m;
	int idx_n;
	int idx_r;
	int idx_c;

	int pp=0;

	for(int row=0; row<R; row+=Tr){
		for(int col =0; col<C; col+=Tc){
			for(int to=0; to<M; to+=Tm){

				//clean_output(conv2d_output,to,idx_m,row,idx_r,col,idx_c,R,C,M,N,output_1);

				for(int ti =0; ti <N; ti+=Tn){
				//==============================================================================

					idx_r=(row+Tr<R)?row+Tr-row:R-row;
					idx_c=(col+Tc<C)?col+Tc-col:C-col;
					idx_m=(to+Tm<M)?to+Tm-to:M-to;
					idx_n=(ti+Tn<N)?ti+Tn-ti:N-ti;





					load_kernel(conv2d_input_2,to,idx_m,ti,idx_n,R,C,M,N,kernel_1);

					load_input(conv2d_input_1,ti,idx_n,row,idx_r,col,idx_c,R,C,M,N,input_1);

					compute(idx_r,idx_c,idx_m,idx_n,output_1,kernel_1,input_1,ti);



					pp=1-pp;
				}

				store_output(conv2d_output,to,idx_m,row,idx_r,col,idx_c,R,C,M,N,output_1);
			}
		}
	}

	return ;
}





















