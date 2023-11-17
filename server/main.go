package main

import (
	"context"
	"log"
	"time"

	"github.com/gin-gonic/gin"
	"go.mongodb.org/mongo-driver/bson"
	"go.mongodb.org/mongo-driver/bson/primitive"
	"go.mongodb.org/mongo-driver/mongo"
	"go.mongodb.org/mongo-driver/mongo/options"
)

func main() {
	ctx, cancel := context.WithCancel(context.Background())
	defer cancel()
	dbClient, err := mongo.Connect(ctx, options.Client().ApplyURI("mongodb://localhost:27017"))
	if err != nil {
		log.Fatal(err)
	}

	sensorDb := dbClient.Database("sensor-server")
	sensorDataCollection := sensorDb.Collection("sensor-data")
	deviceStatusCollection := sensorDb.Collection("device-status")

	deviceStatusId, err := primitive.ObjectIDFromHex("6557b0290fdad606e4a12adb")
	if err != nil {
		log.Fatal(err)
	}

	r := gin.Default()
	r.Use(func(c *gin.Context) {
		c.Header("Content-Type", "application/json")
		c.Next()
	})

	r.NoRoute(func(ctx *gin.Context) { ctx.JSON(404, gin.H{"error": "endpoint not found"}) })
	r.GET("/", func(ctx *gin.Context) { ctx.JSON(200, gin.H{"message": "welcome to sensor server"}) })

	r.GET("/sensor-data", func(c *gin.Context) {
		cursor, err := sensorDataCollection.Find(context.Background(), bson.M{})
		if err != nil {
			c.JSON(500, gin.H{"error": err.Error()})
			return
		}

		var results []bson.M
		if err := cursor.All(context.Background(), &results); err != nil {
			c.JSON(500, gin.H{"error": err.Error()})
			return
		}
		c.JSON(200, results)
	})

	r.GET("/device-status", func(c *gin.Context) {
		ds := &DeviceStatus{}
		err := deviceStatusCollection.FindOne(context.Background(), bson.M{"_id": deviceStatusId}).Decode(ds)
		if err != nil {
			if err == mongo.ErrNoDocuments {
				newDs, err := deviceStatusCollection.InsertOne(context.Background(), &DeviceStatus{Id: deviceStatusId, IsOn: false, WifiConnected: false, UpdatedAt: time.Now()})
				if err != nil {
					c.JSON(500, gin.H{"error": err.Error()})
					return
				}
				c.JSON(200, newDs)
				return
			}
			c.JSON(500, gin.H{"error": err.Error()})
			return
		}
		c.JSON(200, ds)
	})

	r.POST("/sensor-data", func(c *gin.Context) {
		sensorDataJson := struct {
			Temperature float64 `json:"temperature" binding:"required"`
			Humidity    float64 `json:"humidity" binding:"required"`
		}{}
		if err := c.ShouldBindJSON(&sensorDataJson); err != nil {
			c.JSON(400, gin.H{"error": err.Error()})
			return
		}

		sd := &SensorData{
			Humidity:    sensorDataJson.Humidity,
			Temperature: sensorDataJson.Temperature,
			UpdatedAt:   time.Now(),
			Id:          primitive.NewObjectID(),
		}

		if _, err := sensorDataCollection.InsertOne(context.Background(), sd); err != nil {
			c.JSON(500, gin.H{"error": err.Error()})
			return
		}
		c.JSON(200, gin.H{"message": "sensor data saved", "data": sd})
	})

	r.POST("/device-status", func(c *gin.Context) {
		deviceStatusJson := struct {
			IsOn          bool `json:"is_on"`
			WifiConnected bool `json:"wifi_connected"`
		}{}

		if err := c.ShouldBindJSON(&deviceStatusJson); err != nil {
			c.JSON(400, gin.H{"error": err.Error()})
			return
		}

		ds := &DeviceStatus{
			IsOn:          deviceStatusJson.IsOn,
			WifiConnected: deviceStatusJson.WifiConnected,
			UpdatedAt:     time.Now(),
			Id:            deviceStatusId,
		}

		exists := true
		err := deviceStatusCollection.FindOne(context.Background(), bson.M{"_id": ds.Id}).Err()
		if err != nil {
			if err == mongo.ErrNoDocuments {
				exists = false
			} else {
				c.JSON(500, gin.H{"error": err.Error()})
				return
			}
		}

		if !exists {
			_, err := deviceStatusCollection.InsertOne(context.Background(), ds)
			if err != nil {
				c.JSON(500, gin.H{"error": err.Error()})
				return
			}
		} else {
			if _, err := deviceStatusCollection.UpdateOne(context.Background(), bson.M{"_id": ds.Id}, bson.M{"$set": ds}); err != nil {
				c.JSON(500, gin.H{"error": err.Error()})
				return
			}
		}

		c.JSON(200, gin.H{"message": "device status saved", "data": ds})
	})

	r.Run(":8000")
}

type SensorData struct {
	Id          primitive.ObjectID `json:"_id,omitempty" bson:"_id,omitempty"`
	Temperature float64            `json:"temperature" bson:"temperature"`
	Humidity    float64            `json:"humidity" bson:"humidity"`
	UpdatedAt   time.Time          `json:"updated_at" bson:"updated_at"`
}

type DeviceStatus struct {
	Id            primitive.ObjectID `json:"_id,omitempty" bson:"_id,omitempty"`
	IsOn          bool               `json:"is_on" bson:"is_on"`
	WifiConnected bool               `json:"wifi_connected" bson:"wifi_connected"`
	UpdatedAt     time.Time          `json:"updated_at" bson:"updated_at"`
}
