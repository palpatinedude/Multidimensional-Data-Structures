/*
#ifndef POINT3D_H
#define POINT3D_H

#include <string>
#include <iostream>
#include <cmath>
#include "../../json.hpp"

using json = nlohmann::json;

class Point3D {
private:
    float x;          // longitude
    float y;          // latitude
    std::string t;    // Timestamp string (e.g., ISO 8601)

public:
    // Constructors
    Point3D();                                                  // Default constructor (0,0,"")
    Point3D(float x, float y, const std::string& t);
    Point3D(const Point3D& other);
    Point3D(Point3D&& other) noexcept;

    // Assignment
    Point3D& operator=(const Point3D& other);
    Point3D& operator=(Point3D&& other) noexcept;


    // Accessors
    float getX() const;
    float getY() const;
    std::string getT() const;

    // Utility
    void print() const;
    json to_json() const;
    void validate() const;

    // Comparison
    bool operator==(const Point3D& other) const;
    bool operator!=(const Point3D& other) const; 

    // Distance calculations
    float distanceSquaredTo(const Point3D& other) const;  // squared Euclidean distance (no sqrt)
    float distanceTo(const Point3D& other) const;         // Euclidean distance (calls distanceSquaredTo internally)
};

#endif // POINT3D_H
*/
/*
 * point3d.h
 * -----------
 * Defines the Point3D class which represents a 3-dimensional point with:
 * - x: longitude
 * - y: latitude
 * - t: timestamp in ISO 8601 format
 *
 * Provides:
 * - Constructors (default, parameterized, copy, move)
 * - Assignment operators (copy and move)
 * - Accessors for x, y, t
 * - Utilities: printing, JSON conversion, validation
 * - Comparison operators (==, !=)
 * - Distance calculations (Euclidean distance)
 *
 * This header depends on nlohmann::json for JSON serialization.
 */

#ifndef POINT3D_H
#define POINT3D_H

#include <string>
#include <iostream>
#include <cmath>
#include "../../json.hpp"

using json = nlohmann::json;

class Point3D {
private:
    float x;          // Longitude 
    float y;          // Latitude
    std::string t;    // Timestamp (ISO 8601 format)

    void validate() const;      // Validates latitude, longitude, timestamp

public:
    // -------------------- Constructors --------------------
    Point3D();                                            // Default constructor (0,0,"")
    Point3D(float x, float y, const std::string& t);     // Parameterized constructor
    Point3D(const Point3D& other);                       // Copy constructor
    Point3D(Point3D&& other) noexcept;                   // Move constructor

    // -------------------- Assignment --------------------
    Point3D& operator=(const Point3D& other);            // Copy assignment
    Point3D& operator=(Point3D&& other) noexcept;       // Move assignment

    // -------------------- Accessors --------------------
    float getX() const;
    float getY() const;
    std::string getT() const;

    // -------------------- Utilities --------------------
    void print() const;         // Prints the point to stdout
    json to_json() const;       // Converts point to JSON

    // -------------------- Comparison --------------------
    bool operator==(const Point3D& other) const;
    bool operator!=(const Point3D& other) const;

    // -------------------- Distance --------------------
    float distanceTo(const Point3D& other) const;           // Euclidean distance
    float distanceSquaredTo(const Point3D& other) const;    // Squared distance (faster)
};

#endif // POINT3D_H
