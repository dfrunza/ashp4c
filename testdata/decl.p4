control p(in bit y_0)
{
    apply {
        bit x;
        x = 8w1;
    
        if ((int)x == 8w1)
        {
            bit y;
            y = 8w1;
        }
        else
        {
            bit y;
            y = y_0;
            {
                bit y;
                y = 8w1;

                {
                    bit y;
                    y = 8w1;
                }
            }
        }
    }
}
