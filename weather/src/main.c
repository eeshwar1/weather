#include "weather.h"

int main(int argc, char *argv[])
{
       
		WeatherResponse response = getWeather();

		Weather current = response.weather;

		printf("Weather: %s, %s\n",
				current.main, current.description);

		printf("Temp: %.1f C\nFeels Like: %.1f C\nMin: %.1f C\nMax: %.1f C\n",
				current.temp, current.feels_like,
				current.temp_min, current.temp_max);

		printf("Date Time: %s\nSunrise: %s\nSunset: %s\n", 
				format_time(current.dt), format_time(current.sunrise), 
				format_time(current.sunset));
 

		return 0;
}

