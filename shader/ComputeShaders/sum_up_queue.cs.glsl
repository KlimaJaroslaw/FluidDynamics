layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;

uniform int power;
uniform int limit;

void main() {
    uint index = gl_WorkGroupID.x;

    if(queue[index].val<=limit && index>=int(pow(2,float(power)))){
        queue[index].val += queue[index-int(pow(2,float(power)))].val;
    }
}