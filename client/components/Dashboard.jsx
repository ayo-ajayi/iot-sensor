import React, { useState, useEffect, useRef } from "react";
import {
  LineChart,
  Line,
  XAxis,
  YAxis,
  CartesianGrid,
  Tooltip,
  Legend,
} from "recharts";

const BaseUrl = import.meta.env.VITE_BASE_URL;

const fetchData = async (endpoint) => {
  const response = await fetch(
    `${BaseUrl}${endpoint}?_ts=${new Date().getTime()}`
  );
  if (!response.ok) {
    throw new Error(`HTTP error! status: ${response.status}`);
  }
  return await response.json();
};

const Dashboard = () => {
  const [deviceStatus, setDeviceStatus] = useState({
    is_on: false,
    wifi_connected: false,
  });
  const [sensorData, setSensorData] = useState([]);
  const [lastUpdate, setLastUpdate] = useState("");
  const chartContainerRef = useRef(null);

  useEffect(() => {
    const getDeviceStatus = async () => {
      const statusData = await fetchData("device-status");
      setDeviceStatus(statusData);
    };

    const getSensorData = async () => {
      const data = await fetchData("sensor-data");
      const formattedData = data.map((sd) => ({
        ...sd,
        dateTime: `${new Date(sd.updated_at).toLocaleDateString()} ${new Date(
          sd.updated_at
        ).toLocaleTimeString()}`,
      }));
      setSensorData(formattedData);
      setLastUpdate(new Date().toLocaleString());
    };

    getDeviceStatus();
    getSensorData();
    const statusInterval = setInterval(getDeviceStatus, 10000);
    const sensorDataInterval = setInterval(getSensorData, 120000);

    return () => {
      clearInterval(statusInterval);
      clearInterval(sensorDataInterval);
    };
  }, []);

  useEffect(() => {
    const container = chartContainerRef.current;
    if (container) {
      container.scrollLeft = container.scrollWidth;
    }
  }, [sensorData]);

  return (
    <div style={{ padding: "20px" }}>
      <header
        style={{
          textAlign: "center",
          margin: "20px 0",
          fontWeight: "bold",
          fontSize: "2rem",
        }}
      >
        IOT Sensor
      </header>
      <div
        style={{ fontWeight: "bold", fontSize: "1.2rem", textAlign: "center" }}
      >
        <span style={{ color: deviceStatus.is_on ? "green" : "red" }}>
          {deviceStatus.is_on ? "Online" : "Offline"}
        </span>
        {" | "}
        <span style={{ color: deviceStatus.wifi_connected ? "green" : "red" }}>
          {deviceStatus.wifi_connected
            ? "WiFi Connected"
            : "WiFi Not Connected"}
        </span>
      </div>
      <div
        style={{
          padding: "10px 20px",
          borderRadius: "8px",
          boxShadow: "0px 0px 10px rgba(0, 0, 0, 0.05)",
          margin: "20px 0",
          fontSize: "1.2rem",
          fontWeight: "bold",
          textAlign: "center",
        }}
      >
        <p>
          Latest Temperature:{" "}
          {sensorData.length > 0
            ? `${sensorData[sensorData.length - 1].temperature}°C`
            : "Loading..."}
        </p>
        <p>
          Latest Humidity:{" "}
          {sensorData.length > 0
            ? `${sensorData[sensorData.length - 1].humidity}%`
            : "Loading..."}
        </p>
      </div>
      <div
        ref={chartContainerRef}
        style={{ overflowX: "auto", overflowY: "hidden", whiteSpace: "nowrap" }}
        className="graph-wrapper"
      >
        <LineChart
          width={sensorData.length * 80}
          height={400}
          data={sensorData}
          margin={{ top: 5, right: 30, left: 20, bottom: 5 }}
        >
          <CartesianGrid strokeDasharray="3 3" />
          <XAxis
            dataKey="dateTime"
            tick={{ fontSize: 12 }}
            angle={-45}
            textAnchor="end"
            height={80}
          />

          <YAxis
            orientation="right"
            axisLine={true}
            tickCount={6}
          />

          <Tooltip />
          <Legend />
          <Line
            type="monotone"
            dataKey="temperature"
            stroke="#8884d8"
            strokeWidth={2}
          />
          <Line
            type="monotone"
            dataKey="humidity"
            stroke="#82ca9d"
            strokeWidth={2}
          />
        </LineChart>
      </div>

      <footer
        style={{
          textAlign: "center",
          fontSize: "0.9rem",
          color: "#757575",
          marginTop: "20px",
        }}
      >
        Last Updated: {lastUpdate}
        <div>© {new Date().getFullYear()} Ayomide Ajayi</div>
      </footer>
    </div>
  );
};

export default Dashboard;
