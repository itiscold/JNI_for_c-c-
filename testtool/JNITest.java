
class Control{
	public int v;
	public int pos;
	public int ebv;
	public int cmdv;
}

class Wayside{
	public int slope;
	public int speed;
	public int[] data= {0,0,0,0}; 
}

public class JNITest{
	public native static int getGear(Control s, Wayside w); 
	static {
		System.load("E:\\files\\visual studio 2012\\controlDLL\\x64\\Debug\\controlDLL.dll");
	}
	public static void main(String[] args){
		JNITest test = new JNITest();
		Control c = new Control();
		c.v = 113 ;
		c.pos = 11100;
		c.ebv =120;
		c.cmdv = 112;

		Wayside w = new Wayside();
		w.slope = -12;
		w.speed = 140;
		w.data[0] = 3;
		w.data[1] = 5;
		w.data[2] = 7;
		w.data[3] = 8;
		int res = test.getGear(c,w);
		System.out.println(res);
		System.out.println(c.v);
	}
}