
package com.mycompany.task_e;

import com.google.gson.Gson;
import com.google.gson.reflect.TypeToken;
import java.io.FileWriter;
import java.io.IOException;
import java.lang.reflect.Type;
import java.util.List;

public class SaveSkylineToJSON {
    public static void saveSkylinePointsToJsonFile(List<Point> skylinePoints, String filePath) {
        // Create a Gson object
        Gson gson = new Gson();

        // Convert the list of points to JSON
        Type type = new TypeToken<List<Point>>() {}.getType();
        String json = gson.toJson(skylinePoints, type);

        // Write JSON string to a file
        try (FileWriter writer = new FileWriter(filePath)) {
            writer.write(json);
        } catch (IOException e) {
            e.printStackTrace();
        }
    }
}