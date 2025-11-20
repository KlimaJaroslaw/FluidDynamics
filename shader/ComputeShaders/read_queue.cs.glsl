layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;

uniform vec3 bound_max;
uniform vec3 bound_min;
uniform int limit;

void main() {
    uint index = gl_WorkGroupID.x;
    if(particle[index].type==1){
        if(queue[index].val<limit)
        {
            particle[index].pos = (bound_max-bound_min)/2;
            particle[index].type=0;
            particle[index].vel = vec3(0,0,0);
            const float jitter = 0.5;
            particle[index].pos += hash3(floatBitsToInt(vec3(index,index*1.73,index*7))) * jitter - 0.5 * jitter;
            queue[index].val=0;
        }
        else{
            queue[index].val=1;
        }
    }
    else{
        queue[index].val=0;
    }

}