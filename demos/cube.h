
Object createBox(int w,int h,int dir)
{
	Point p1,p2,p3,p4,p5,p6,p7,p8;
	Quad q1,q2,q3,q4,q5,q6;
	Object b;
	float a,d;

	a=w/2; d=h/2;

	b.begin();
		p1=new Point (-a,-a,-d);
		p2=new Point (-a,+a,-d);
		p3=new Point (+a,+a,-d);
		p4=new Point (+a,-a,-d);
		p5=new Point (-a,-a,+d);
		p6=new Point (-a,+a,+d);
		p7=new Point (+a,+a,+d);
		p8=new Point (+a,-a,+d);

		q1=new Quad (p1,p2,p3,p4);   // front 
		q2=new Quad (p7,p6,p5,p8);	  // back
		q3=new Quad (p6,p2,p1,p5);	  // left
		q4=new Quad (p3,p7,p8,p4);	  // right
		q5=new Quad (p3,p2,p6,p7);   // up
		q6=new Quad (p4,p8,p5,p1);   // down

		if (dir==1) {
			q1.flip();
			q2.flip();
			q3.flip();
			q4.flip();
			q5.flip();
			q6.flip();
		}	

	b.setOrigin(0,0,0);
	b.end();


	return b;
}

Object createRoom(int w, int h)
{
	return createBox(w,h,1);
}	

Object createCube(int w, int h)
{
	return createBox(w,h,0);
}	

