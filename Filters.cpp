const int NCoef = 12;
const int NCoef10 = 10;
const int NCoef8 = 8;
const int NCoefLPF = 9;

/*
 Filter type: Band Pass
 Filter model: Bessel
 Filter order: 6
 Sampling Frequency: 12 KHz
 Fc1 and Fc2 Frequencies: 1.727000 KHz and 1.827000 KHz
*/
//frontend BPF TA for Fdial=136000
float iir1777_12k(float NewSample) {
    double ACoef[NCoef+1] = {
        0.00000001025111874156,
        0.00000000000000000000,
        -0.00000006150671244935,
        0.00000000000000000000,
        0.00000015376678112337,
        0.00000000000000000000,
        -0.00000020502237483116,
        0.00000000000000000000,
        0.00000015376678112337,
        0.00000000000000000000,
        -0.00000006150671244935,
        0.00000000000000000000,
        0.00000001025111874156
    };

    double BCoef[NCoef+1] = {
        1.00000000000000000000,
        -7.03239853541994630000,
        26.37317173204974100000,
        -66.00167721063896000000,
        121.40333205883249000000,
        -171.13625695750076000000,
        189.06543006419187000000,
        -164.55165142689231000000,
        112.24089176526057000000,
        -58.67267609623977600000,
        22.54260574584294100000,
        -5.77971826548247950000,
        0.79025510626735185000
    };

    static double y[NCoef+1]; //output samples
    static double x[NCoef+1]; //input samples
    int n;

    //shift the old samples
    for(n=NCoef; n>0; n--) {
       x[n] = x[n-1];
       y[n] = y[n-1];
    }

    //Calculate the new output
    x[0] = NewSample;
    y[0] = ACoef[0] * x[0];
    for(n=1; n<=NCoef; n++)
        y[0] += ACoef[n] * x[n] - BCoef[n] * y[n];

    return y[0];
}

/*
Filter type: Band Pass
Filter model: Bessel
Filter order: 4
Sampling Frequency: 12 KHz
Fc1 and Fc2 Frequencies: 0.122000 KHz and 0.222000 KHz
Coefficents Quantization: float
*/
//frontend BPF EU for Fdial=136000

float iir172_12k(float NewSample) {
    double ACoef[NCoef8+1] = {
        0.00000057168322210398,
        0.00000000000000000000,
        -0.00000228673288841593,
        0.00000000000000000000,
        0.00000343009933262389,
        0.00000000000000000000,
        -0.00000228673288841593,
        0.00000000000000000000,
        0.00000057168322210398
    };

    double BCoef[NCoef8+1] = {
        1.00000000000000000000,
        -7.80868139726844030000,
        26.70544859120307300000,
        -52.24558806972672200000,
        63.95061366818467300000,
        -50.15166217814823600000,
        24.60776804913673000000,
        -6.90698558141722070000,
        0.84908692083371751000
    };

    static double y[NCoef8+1]; //output samples
    static double x[NCoef8+1]; //input samples
    int n;

    //shift the old samples
    for(n=NCoef8; n>0; n--) {
       x[n] = x[n-1];
       y[n] = y[n-1];
    }

    //Calculate the new output
    x[0] = NewSample;
    y[0] = ACoef[0] * x[0];
    for(n=1; n<=NCoef8; n++)
        y[0] += ACoef[n] * x[n] - BCoef[n] * y[n];
    
    return y[0];
}

/*
  Filter type: Band Pass
  Filter model: Bessel
  Filter order: 6
  Sampling Frequency: 12 KHz
  Fc1 and Fc2 Frequencies: 2.227000 KHz and 2.327000 KHz
*/
// frontend bpf TA for Fdial=135500
float iir2277_12k(float NewSample) {
    double ACoef[NCoef+1] = {
        0.00000002395182760930,
        0.00000000000000000000,
        -0.00000014371096565579,
        0.00000000000000000000,
        0.00000035927741413948,
        0.00000000000000000000,
        -0.00000047903655218597,
        0.00000000000000000000,
        0.00000035927741413948,
        0.00000000000000000000,
        -0.00000014371096565579,
        0.00000000000000000000,
        0.00000002395182760930
    };

    double BCoef[NCoef+1] = {
        1.00000000000000000000,
        -4.35000917874934510000,
        13.65079716026593300000,
        -28.52680808165051500000,
        48.31712537102188100000,
        -63.37681322377901000000,
        69.59752727546485600000,
        -60.93843333782076900000,
        44.67072091143353400000,
        -25.35919858166336000000,
        11.66813679933554000000,
        -3.57514258877210270000,
        0.79025510625003414000
    };

    static double y[NCoef+1]; //output samples
    static double x[NCoef+1]; //input samples
    int n;

    //shift the old samples
    for(n=NCoef; n>0; n--) {
       x[n] = x[n-1];
       y[n] = y[n-1];
    }

    //Calculate the new output
    x[0] = NewSample;
    y[0] = ACoef[0] * x[0];
    for(n=1; n<=NCoef; n++)
        y[0] += ACoef[n] * x[n] - BCoef[n] * y[n];
    
    return y[0];
}

////////////////////////////////////////////////////////////////////////////////
/*
Filter type: Band Pass
Filter model: Bessel
Filter order: 6
Sampling Frequency: 12 KHz
Fc1 and Fc2 Frequencies: 2.727000 KHz and 2.827000 KHz
*/
//frontend BPF TA for Fdial=135000
float iir2777_12k(float NewSample) {
    double ACoef[NCoef+1] = {
        0.00000005059014244448,
        0.00000000000000000000,
        -0.00000030354085466691,
        0.00000000000000000000,
        0.00000075885213666727,
        0.00000000000000000000,
        -0.00000101180284888970,
        0.00000000000000000000,
        0.00000075885213666727,
        0.00000000000000000000,
        -0.00000030354085466691,
        0.00000000000000000000,
        0.00000005059014244448
    };

    double BCoef[NCoef+1] = {
        1.00000000000000000000,
        -1.37117388526208540000,
        6.54938764076864820000,
        -6.82791392548576950000,
        16.90880671289934200000,
        -13.36079185187580600000,
        22.18438002246990600000,
        -12.84676553576459000000,
        15.63282520728423300000,
        -6.06976496615076400000,
        5.59819625014578830000,
        -1.12692685290000720000,
        0.79025510623690898000
    };

    static double y[NCoef+1]; //output samples
    static double x[NCoef+1]; //input samples
    int n;

    //shift the old samples
    for(n=NCoef; n>0; n--) {
       x[n] = x[n-1];
       y[n] = y[n-1];
    }

    //Calculate the new output
    x[0] = NewSample;
    y[0] = ACoef[0] * x[0];
    for(n=1; n<=NCoef; n++)
        y[0] += ACoef[n] * x[n] - BCoef[n] * y[n];
    
    return y[0];
}

/*
Filter type: Band Pass
Filter model: Bessel
Filter order: 5
Sampling Frequency: 12 KHz
Fc1 and Fc2 Frequencies: 0.622000 KHz and 0.722000 KHz
*/
// frontend bpf EU for Fdial=135500

float iir672_12k(float NewSample) {
    double ACoef[NCoef10+1] = {
        0.00000001622206398449,
        0.00000000000000000000,
        -0.00000008111031992243,
        0.00000000000000000000,
        0.00000016222063984487,
        0.00000000000000000000,
        -0.00000016222063984487,
        0.00000000000000000000,
        0.00000008111031992243,
        0.00000000000000000000,
        -0.00000001622206398449
    };

    double BCoef[NCoef10+1] = {
        1.00000000000000000000,
        -9.20444092252574820000,
        38.69270258011452100000,
        -97.76048384272961300000,
        164.33826800599459000000,
        -192.01322896518360000000,
        157.90779641074877000000,
        -90.25962537993488900000,
        34.32618157256693800000,
        -7.84623963002582500000,
        0.81909452823029194000
    };

    static double y[NCoef10+1]; //output samples
    static double x[NCoef10+1]; //input samples
    int n;

    //shift the old samples
    for(n=NCoef10; n>0; n--) {
       x[n] = x[n-1];
       y[n] = y[n-1];
    }

    //Calculate the new output
    x[0] = NewSample;
    y[0] = ACoef[0] * x[0];
    for(n=1; n<=NCoef10; n++)
        y[0] += ACoef[n] * x[n] - BCoef[n] * y[n];
    
    return y[0];
}

/*
Filter type: Band Pass
Filter model: Bessel
Filter order: 6
Sampling Frequency: 12 KHz
Fc1 and Fc2 Frequencies: 1.122000 KHz and 1.222000 KHz
*/
//frontend BPF EU for Fdial=135000
float iir1172_12k(float NewSample) {
    double ACoef[NCoef+1] = {
        0.00000000084286237401,
        0.00000000000000000000,
        -0.00000000505717424405,
        0.00000000000000000000,
        0.00000001264293561012,
        0.00000000000000000000,
        -0.00000001685724748016,
        0.00000000000000000000,
        0.00000001264293561012,
        0.00000000000000000000,
        -0.00000000505717424405,
        0.00000000000000000000,
        0.00000000084286237401
    };

    double BCoef[NCoef+1] = {
        1.00000000000000000000,
        -9.62252652914133220000,
        44.34847108135301900000,
        -128.75242262360811000000,
        261.46057522730729000000,
        -390.55184532958054000000,
        439.58341291129750000000,
        -375.52460802047284000000,
        241.72736888030079000000,
        -114.45515352461790000000,
        37.90702010735147100000,
        -7.90846708438130010000,
        0.79025510628971651000
    };

    static double y[NCoef+1]; //output samples
    static double x[NCoef+1]; //input samples
    int n;

    //shift the old samples
    for(n=NCoef; n>0; n--) {
       x[n] = x[n-1];
       y[n] = y[n-1];
    }

    //Calculate the new output
    x[0] = NewSample;
    y[0] = ACoef[0] * x[0];
    for(n=1; n<=NCoef; n++)
        y[0] += ACoef[n] * x[n] - BCoef[n] * y[n];
    
    return y[0];
}


/*
Filter type: Band Pass
Filter model: Bessel
Filter order: 6
Sampling Frequency: 12 KHz
Fc1 and Fc2 Frequencies: 0.727000 KHz and 0.827000 KHz
*/
//frontend BPF TA for Fdial=137000
float iir777_12k(float NewSample) {
    double ACoef[NCoef+1] = {
        0.00000000055821951372,
        0.00000000000000000000,
        -0.00000000334931708233,
        0.00000000000000000000,
        0.00000000837329270583,
        0.00000000000000000000,
        -0.00000001116439027443,
        0.00000000000000000000,
        0.00000000837329270583,
        0.00000000000000000000,
        -0.00000000334931708233,
        0.00000000000000000000,
        0.00000000055821951372
    };

    double BCoef[NCoef+1] = {
        1.00000000000000000000,
        -10.80929767711960600000,
        54.45230841404415400000,
        -168.90449411933292000000,
        359.10400867016432000000,
        -551.09903116202520000000,
        625.84833094127748000000,
        -529.89419502665862000000,
        332.00111692525934000000,
        -150.14843717237403000000,
        46.54329038088767600000,
        -8.88383883664151950000,
        0.79025510630651841000
    };

    static double y[NCoef+1]; //output samples
    static double x[NCoef+1]; //input samples
    int n;

    //shift the old samples
    for(n=NCoef; n>0; n--) {
       x[n] = x[n-1];
       y[n] = y[n-1];
    }

    //Calculate the new output
    x[0] = NewSample;
    y[0] = ACoef[0] * x[0];
    for(n=1; n<=NCoef; n++)
        y[0] += ACoef[n] * x[n] - BCoef[n] * y[n];
    
    return y[0];
}

/*
Filter type: Band Pass
Filter model: Bessel
Filter order: 6
Sampling Frequency: 12 KHz
Fc1 and Fc2 Frequencies: 0.778000 KHz and 0.878000 KHz
*/
//frontend BPF EU for Fdial=137000
float iir828_12k(float NewSample) {
    double ACoef[NCoef+1] = {
        0.00000000049005409105,
        0.00000000000000000000,
        -0.00000000294032454633,
        0.00000000000000000000,
        0.00000000735081136582,
        0.00000000000000000000,
        -0.00000000980108182109,
        0.00000000000000000000,
        0.00000000735081136582,
        0.00000000000000000000,
        -0.00000000294032454633,
        0.00000000000000000000,
        0.00000000049005409105
    };

    double BCoef[NCoef+1] = {
        1.00000000000000000000,
        -10.68108820225567600000,
        53.30421676515713600000,
        -164.17573622969641000000,
        347.32324156346436000000,
        -531.45598292756836000000,
        602.95390706856460000000,
        -511.00698274643395000000,
        321.10951140601458000000,
        -145.94479683605095000000,
        45.56195730384600000000,
        -8.77846729942790740000,
        0.79025510630227869000
    };

    static double y[NCoef+1]; //output samples
    static double x[NCoef+1]; //input samples
    int n;

    //shift the old samples
    for(n=NCoef; n>0; n--) {
       x[n] = x[n-1];
       y[n] = y[n-1];
    }

    //Calculate the new output
    x[0] = NewSample;
    y[0] = ACoef[0] * x[0];
    for(n=1; n<=NCoef; n++)
        y[0] += ACoef[n] * x[n] - BCoef[n] * y[n];
    
    return y[0];
}


/*
  Filter type: Low Pass
  Filter model: Bessel
  Filter order: 9
  Sampling Frequency: 12 KHz
  Cut Frequency: 0.200000 KHz
*/
// LPF after first mixer

float iirlpf200_12k(float NewSample) {
    double ACoef[NCoefLPF+1] = {
        0.00000000000469045100,
        0.00000000004221405900,
        0.00000000016885623602,
        0.00000000039399788404,
        0.00000000059099682605,
        0.00000000059099682605,
        0.00000000039399788404,
        0.00000000016885623602,
        0.00000000004221405900,
        0.00000000000469045100
    };

    double BCoef[NCoefLPF+1] = {
        1.00000000000000000000,
        -8.32029282259674300000,
        30.78653165799518400000,
        -66.49081272375461500000,
        92.37083795883745600000,
        -85.59941855927252200000,
        52.91338946286465000000,
        -21.03880905618428800000,
        4.88243156011711130000,
        -0.50385747692091765000
    };

    static double y[NCoefLPF+1]; //output samples
    static double x[NCoefLPF+1]; //input samples
    int n;

    //shift the old samples
    for(n=NCoefLPF; n>0; n--) {
       x[n] = x[n-1];
       y[n] = y[n-1];
    }

    //Calculate the new output
    x[0] = NewSample;
    y[0] = ACoef[0] * x[0];
    for(n=1; n<=NCoefLPF; n++)
        y[0] += ACoef[n] * x[n] - BCoef[n] * y[n];
    
    return y[0];
}
