
package com.mycompany.task_e;

import java.util.Random;


public class Point {
    int x;
    int y;

    public Point(int x, int y) {
        this.x = x;
        this.y = y;
    }

    public boolean dominates(Point other) 
    {
        return this.x >= other.x && this.y >= other.y;
    }
    
    public int getX() {
        return x;
    }

    public int getY() {
        return y;
    }

    
    
    public void setX(int x) {
        this.x = x;
    }

    public void setY(int y) {
        this.y = y;
    }

    @Override
    public String toString() 
    {
        return "(" + x + ", " + y + ")";
    }
}
