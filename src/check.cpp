#include <string.h>
#include <iostream>
#include <time.h>
#include <stdlib.h>

using namespace std;

const char * data = "00100100*00#28579930097403522305074661398381831902838308390297240073945143394457554382709012379133963268839821691856993542845264357596146786081577683858735602313228774016568284842774912936345869149208484977244133442850208619145874008585746646383934581375386469689552118753091809520667480751708450006103349476987171034967168096188908173616294514035918108962230793074767816541031404944241671226884804975800321851976005468789150817927748033746838743846650731167527218747339743455209419082812514928527017823140022330040500042255";
bool check_crc(void){
    char  scrc[5]= "";
    char sint[2] = "";
    const char * pos;
    int crc,total=0;
    int len = strlen(data);
    sint[1]= '\0';

    pos = data + (len -4);
    strncpy(scrc,pos,5);
    crc = atoi(scrc);

    len = len -22;
    for(int i=12; i< len;i++){
	sint[0] = data[i]; 
	total = total + atoi(sint);
    }

    return crc ==  total;
};

time_t extract_date(void){
    int lenc,posc,lenb,posb,lena,posa;
    char tmp[4] = "";
    const char * pos;
    int len = strlen(data);

    pos = data + (len - 7);
    strncpy(tmp,pos,3);
    lenc = atoi(tmp);

    pos = data + (len - 10);
    strncpy(tmp,pos,3);
    posc = atoi(tmp);

    pos = data + (len - 13);
    strncpy(tmp,pos,3);
    lenb = atoi(tmp);

    pos = data + (len - 16);
    strncpy(tmp,pos,3);
    posb = atoi(tmp);

    pos = data + (len - 19);
    strncpy(tmp,pos,3);
    lena = atoi(tmp);

    pos = data + (len - 22);
    strncpy(tmp,pos,3);
    posa = atoi(tmp);

    char tmp2[24] = "";
    posa += 12;
    posb += 12;
    posc += 12;

    int c=0;
    for(int i=posa;i<posa+lena;i++,c++){
	tmp2[c]=data[i];
    }

    for(int i=posb;i<posb+lenb;i++,c++){
	tmp2[c]=data[i];
    }

    for(int i=posc;i<posc+lenc;i++,c++){
	tmp2[c]=data[i];
    }

    tmp2[c]= '\0';
    int r = atoi(tmp2);
    return r;
};

void check_key(void){
    time_t fin,now;
    if(!check_crc())
	exit(0);
    fin = extract_date();
    now = time(0);
    if(now > fin)
	exit(0);
};

