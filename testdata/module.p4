parser f(out bool x)
{ state start { transition accept; } }

parser f2(out bool x, out bool y)
{ state start { transition accept; } }

parser Filter(out bool filter);

typedef bool T;
parser Extractor(out T dt);

typedef bool T1;
typedef bool T2;
parser Extractor2(out T1 data1, out T2 data2);

package s();
package switch0(Filter f);
package switch1(Extractor e);
package switch2(Extractor e);
package switch3(Extractor2 e);
package switch4(Extractor2 e);

switch0(f()) main1;
switch1(f()) main3;
switch2(f()) main4;
switch3(f2()) main5;
switch4(f2()) main6;
switch1(f()) main2;
switch4(f2()) main7;
