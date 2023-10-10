class Score {
public:
	String history="";
	int Fh[10];
	int Th[10];
	int Ch[10];
	int insert=0;
	String comma=", ";
	int alert=0;
	int CTup=0;
	int Fup=0;
	int level=0;
	
	void begin() {
	}

	float av(int a[], int j, int count) { // target, begin, count
		float sum=0;
		for (int i=j; i<j+count; i++)
			sum += a[i];
		return sum/count;
	}
	
	//        F를 0부터 값으로 환산.
	int score(int F, int T, int C) {
		if (insert<10) {
			Fh[insert]=F;
			Th[insert]=T;
			Ch[insert]=C;
			insert++;
			return 0;
		} else {
			for (int i=1;i<10; i++) {
				Fh[i-1]=Fh[i];
				Th[i-1]=Th[i];
				Ch[i-1]=Ch[i];
				Fh[9]=F;
				Th[9]=T;
				Ch[9]=C;
			}
		}
		
		level=0;
		bool Tb=false, Cb=false;
		history="";
		if (Fh[8]>0 && F>0) {
			level+=1;  // IR에 2회연속 무언가라도 잡히면 +1
			if (F>3000) {
				history += comma + "IR-2초이상-높은값-검출";
				level+=5;
			}
			else history += comma + "IR-2초이상-미상의값-검출";
			comma = ", ";
		}
		if (av(Fh,0,3) && av(Fh,7,3)>av(Fh,4,3) && av(Fh,3,3)>av(Fh,0,3)) { // 상승세인경우 상승비율
			int up = int(av(Fh,6,3)/av(Fh,0,3) +0.5);
			if (F>3000 && up>100) {
				level=10;
				history += comma + "IR-급상승("+ String(Fup)+"초)";
			} else {
				level += up;
				if (level>9) level=9;
				history += comma + "IR-상승("+ String(Fup)+"초)";
			}
			Fup++;
			history += comma + "IR-상승("+ String(Fup)+"초)";
			comma = ", ";
		} else if (Fup>0) Fup--;

		if (av(Th,7,3)>av(Th,4,3) && av(Th,3,3)>av(Th,0,3)) Tb=true; //온도 상승세인경우 상승비율
		if (av(Ch,7,3)>av(Ch,4,3) && av(Ch,3,3)>av(Ch,0,3)) Cb=true; //CO2 상승세
		
		if (Tb && Cb) {
			CTup++;
			history += comma + "온도/CO2-동시상승("+ String(CTup)+"초)";
			comma = ", ";
			if (av(Fh,0,3)) level += int(av(Th,7,3)/av(Th,4,3) +0.5) + int(av(Ch,4,3)/av(Ch,4,3) +0.5);
		} else if (CTup>0) CTup--;
		
		if (F != 0 && Fh[7]==Fh[8] && Fh[8]==Fh[9] && !Tb && !Cb) { // IR값이 0이 아닌데, 3연속 변경없고, 온도, CO2변경없음
			history += comma + "IR센서-이상-의심(온도정상-CO2정상)";
			comma = ", ";
		}
		
		if (F>1000 && Tb && T>40 && Cb && C>2000) {
			level = 10;
			history = "화재의심: 높은 IR, 높은 온도, 높은 CO2";
		}
		
		if (level==0 && history != "") {
			if (alert++ > 30) {
				alert=0;
				history="";
			}
		}
		return level;
	}
};