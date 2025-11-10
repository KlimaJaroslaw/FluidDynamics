layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;

void main() {
    uint index = gl_WorkGroupID.x;
    //This drains up the water
    if (particle[index].pos.z <-0.6 && particle[index].pos.x < -0.6 && particle[index].pos.y < -0.6) {
        particle[index].type=1;
        queue[index].val=1;
    }
}