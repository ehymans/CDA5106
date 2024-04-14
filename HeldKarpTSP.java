import java.util.ArrayList;
import java.util.List;

public class HeldKarpTSP {
    public static List<Point> cities;

    public static class Point {
        public double x;
        public double y;

        public Point(double x, double y) {
            this.x = x;
            this.y = y;
        }

        @Override
        public String toString() {
            return "(" + x + ", " + y + ")";
        }
    }

    public static double distance(Point p1, Point p2) {
        double dx = p1.x - p2.x;
        double dy = p1.y - p2.y;
        return Math.sqrt(dx * dx + dy * dy);
    }

    public static List<Point> heldKarp(int n, List<Point> cities) 
    {
        double[][] dp = new double[1 << n][n];
        int[][] prev = new int[1 << n][n];

        for (int mask = 0; mask < (1 << n); mask++) 
        {
            for (int i = 0; i < n; i++) 
            {
                dp[mask][i] = Double.POSITIVE_INFINITY;
            }
        }

        dp[1][0] = 0;

        for (int mask = 1; mask < (1 << n); mask++) 
        {
            for (int i = 0; i < n; i++) 
            {
                if ((mask & (1 << i)) != 0) 
                {
                    for (int j = 0; j < n; j++) 
                    {
                        if (i != j && (mask & (1 << j)) != 0) 
                        {
                            double newDist = dp[mask ^ (1 << i)][j] + distance(cities.get(j), cities.get(i));
                            if (newDist < dp[mask][i]) 
                            {
                                dp[mask][i] = newDist;
                                prev[mask][i] = j;
                            }
                        }
                    }
                }
            }
        }

        int lastMask = (1 << n) - 1;
        int lastCity = 0;
        double minTourCost = Double.POSITIVE_INFINITY;

        for (int i = 1; i < n; i++) 
        {
            double tourCost = dp[lastMask][i] + distance(cities.get(i), cities.get(0));
            if (tourCost < minTourCost) 
            {
                minTourCost = tourCost;
                lastCity = i;
            }
        }

        List<Point> optimalTour = new ArrayList<>();
        int currentMask = lastMask;
        int currentCity = lastCity;

        while (currentMask != 1) 
        {
            optimalTour.add(0, cities.get(currentCity));
            int nextCity = prev[currentMask][currentCity];
            currentMask ^= (1 << currentCity);
            currentCity = nextCity;
        }

        optimalTour.add(0, cities.get(0));
        optimalTour.add(cities.get(0));

        return optimalTour;
    }

    public static void main(String[] args) {
        cities = new ArrayList<>();
        cities.add(new Point(5.62197706436568, 50.43553223887989));
        cities.add(new Point(73.69001370749064, 13.126699134586694));
        cities.add(new Point(5.683407576617205, 19.19880990529429));
        cities.add(new Point(94.53768262203981, 63.66857631372882));
        cities.add(new Point(81.42885384544759, 65.49545511132014));
        cities.add(new Point(76.32035414534141, 99.14674007307472));
        cities.add(new Point(51.39574229380559, 1.1652895313832623));
        cities.add(new Point(25.859614551298026, 57.39669295697078));
        cities.add(new Point(67.58113008138884, 10.102475685008605));
        cities.add(new Point(0.35138532294655134, 10.398404434718655));
        cities.add(new Point(61.94415359823191, 96.03503559449332));
        cities.add(new Point(8.587973090377176, 39.7662397016992));
        cities.add(new Point(95.8176557640272, 83.27844094904448));
        cities.add(new Point(87.97900622388883, 16.288664488529335));
        cities.add(new Point(1.978158686562137, 51.93598153615081));
        
        Point origin = new Point(82.32547314252422, 86.52371506977595);
        cities.add(0, origin);

        Runtime runtime = Runtime.getRuntime();
        long usedMemoryBefore = runtime.totalMemory() - runtime.freeMemory();

        long startTime = System.nanoTime();
        List<Point> optimalTour = heldKarp(cities.size(), cities);
        long endTime = System.nanoTime();
        long executionTime = endTime - startTime;

        long usedMemoryAfter = runtime.totalMemory() - runtime.freeMemory();
        long totalMemoryUsed = usedMemoryAfter - usedMemoryBefore;

        double minTourCost = 0;

        System.out.println("Optimal Path:");
        for (int i = 0; i < optimalTour.size(); i++) {
            Point city = optimalTour.get(i);
            System.out.print(city);
            if (i < optimalTour.size() - 1) {
                System.out.print(" -> ");
                minTourCost += distance(city, optimalTour.get(i + 1));
            }
        }
        System.out.println();

        System.out.println("Minimum Tour Cost: " + minTourCost);
        System.out.println("Execution Time: " + executionTime + " nanoseconds");
        System.out.println("Total Memory Used: " + totalMemoryUsed + " bytes");
    }

}
/* 

// RANDOM GENERATED VERSION OF HeldKarpTSP.java 

import java.util.ArrayList;
import java.util.List;
import java.util.Random;

public class HeldKarpTSP 
{
    public static double gridX = 100.0;
    public static double gridY = 100.0;
    public static int numCities = 15;
    public static List<Point> cities;
    public static Random r = new Random();

    public static class Point 
    {
        public double x;
        public double y;

        public Point(double x, double y) 
        {
            this.x = x;
            this.y = y;
        }

        @Override
        public String toString() 
        {
            return "(" + x + ", " + y + ")";
        }
    }

    public static List<Point> generateCities(int n) 
    {
        List<Point> cities = new ArrayList<>();

        for (int i = 0; i < n; i++) 
        {
            double x = r.nextDouble() * gridX;
            double y = r.nextDouble() * gridY;
            cities.add(new Point(x, y));
        }

        return cities;
    }

    public static double distance(Point p1, Point p2) 
    {
        double dx = p1.x - p2.x;
        double dy = p1.y - p2.y;
        return Math.sqrt(dx * dx + dy * dy);
    }

    public static List<Point> heldKarp(int n, List<Point> cities) 
    {
        double[][] dp = new double[1 << n][n];
        int[][] prev = new int[1 << n][n];

        for (int mask = 0; mask < (1 << n); mask++) 
        {
            for (int i = 0; i < n; i++) 
            {
                dp[mask][i] = Double.POSITIVE_INFINITY;
            }
        }

        dp[1][0] = 0;

        for (int mask = 1; mask < (1 << n); mask++) 
        {
            for (int i = 0; i < n; i++) 
            {
                if ((mask & (1 << i)) != 0) 
                {
                    for (int j = 0; j < n; j++) 
                    {
                        if (i != j && (mask & (1 << j)) != 0) 
                        {
                            double newDist = dp[mask ^ (1 << i)][j] + distance(cities.get(j), cities.get(i));
                            if (newDist < dp[mask][i]) 
                            {
                                dp[mask][i] = newDist;
                                prev[mask][i] = j;
                            }
                        }
                    }
                }
            }
        }

        int lastMask = (1 << n) - 1;
        int lastCity = 0;
        double minTourCost = Double.POSITIVE_INFINITY;

        for (int i = 1; i < n; i++) 
        {
            double tourCost = dp[lastMask][i] + distance(cities.get(i), cities.get(0));
            if (tourCost < minTourCost) 
            {
                minTourCost = tourCost;
                lastCity = i;
            }
        }

        List<Point> optimalTour = new ArrayList<>();
        int currentMask = lastMask;
        int currentCity = lastCity;

        while (currentMask != 1) 
        {
            optimalTour.add(0, cities.get(currentCity));
            int nextCity = prev[currentMask][currentCity];
            currentMask ^= (1 << currentCity);
            currentCity = nextCity;
        }

        optimalTour.add(0, cities.get(0));
        optimalTour.add(cities.get(0));

        return optimalTour;
    }

    public static void main(String[] args) 
    {
        cities = generateCities(numCities);
        Point origin = new Point(r.nextDouble() * gridX, r.nextDouble() * gridY);
        cities.add(0, origin);

        List<Point> optimalTour = heldKarp(numCities + 1, cities);
        double minTourCost = 0;

        for (int i = 0; i < optimalTour.size() - 1; i++) 
        {
            minTourCost += distance(optimalTour.get(i), optimalTour.get(i + 1));
        }

        System.out.println("Optimal Tour:");
        for (Point city : optimalTour) 
        {
            System.out.println(city);
        }

        System.out.println("Minimum Tour Cost: " + minTourCost);
    }
}*/