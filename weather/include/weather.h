#ifndef __WEATHER_H_
#define __WEATHER_H_

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <curl/curl.h>
#include <string.h>
#include <cjson/cJSON.h>
#include <conversions.h>

typedef struct  {
  char *memory;
  size_t size;
  bool error;
  char *message;
} MemoryStruct;

typedef struct  {

	char   main[50];
	char   description[100];
	double temp;
	double feels_like;
	double temp_max;
	double temp_min;
	int64_t sunrise;
	int64_t sunset;
	int64_t dt;
	int clouds;
	int visibility;

} Weather;

typedef struct {

	Weather weather;
	char *message;
} WeatherResponse;

static size_t
WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
  size_t realsize = size * nmemb;
  
  MemoryStruct *mem = (MemoryStruct *)userp;
 
  char *ptr = realloc(mem->memory, mem->size + realsize + 1);
  if(!ptr) {
    /* out of memory! */
    printf("not enough memory (realloc returned NULL)\n");
    return 0;
  }
 
  mem->memory = ptr;
  memcpy(&(mem->memory[mem->size]), contents, realsize);
  mem->size += realsize;
  mem->memory[mem->size] = 0;
 
  return realsize;
}

WeatherResponse getWeather(); 
Weather extractData(cJSON *jsonData);
MemoryStruct getAPIData();
char* format_time(int datetime);


WeatherResponse getWeather() {

		MemoryStruct chunk = getAPIData();

		if (chunk.error == true) 
		{

			printf("ERROR: %s\n", chunk.message);
			exit(1);

		}
        printf("Response message: %s\n", chunk.message);

//		printf("Response: %s\n", chunk.memory);
        cJSON *json = cJSON_Parse(chunk.memory);


        if (json == NULL)
        {
                const char *error_ptr = cJSON_GetErrorPtr();
                if (error_ptr != NULL)
                {
                        fprintf(stderr, "Error before: %s\n", error_ptr);
                }
                exit(1);
        }

        Weather current = extractData(json);

		WeatherResponse response = { .weather = current, 
			.message = chunk.message };

		return response;


}
MemoryStruct getAPIData()
{


        CURL *curl;
        CURLcode res;

        char *lat="32.93";
        char *lon="-96.87";
        char *apiKey="938f1f75c78441cd9db12ad01c213146";

        char url[250];

        snprintf(url, 250, 
   "https://api.openweathermap.org/data/2.5/weather?lat=%s&lon=%s&appid=%s", 
        lat, lon, apiKey);
  
        MemoryStruct chunk;
 
        chunk.memory = malloc(1);  /* grown as needed by the realloc above */
        chunk.size = 0;    /* no data at this point */

        curl = curl_easy_init();
        if(curl) 
        {
        
	        curl_easy_setopt(curl, CURLOPT_URL, url);

		    /* send all data to this function  */
			curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, 
					WriteMemoryCallback);

			/* we pass our 'chunk' struct to the callback function */
			curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);

			/* some servers do not like requests that are made without a 
			 * user-agent field, so we provide one */
			curl_easy_setopt(curl, CURLOPT_USERAGENT, "libcurl-agent/1.0");

			res = curl_easy_perform(curl);
		    curl_easy_cleanup(curl);
    

			/* check for errors */
			if(res != CURLE_OK)
			{
                fprintf(stderr, "curl_easy_perform() failed: %s\n",
                        curl_easy_strerror(res));
				chunk.error = true;
				chunk.message = "Error getting data";
			}	
			else
			{

				chunk.error = false;
				chunk.message = "Data retrieved successfully";
				return chunk;
			}
        }
		else
		{

			chunk.error = true;
			chunk.message = "Error creating request";

		}

		return chunk;

}

Weather extractData(cJSON *jsonData)
{

    const cJSON *name = NULL;
    name = cJSON_GetObjectItemCaseSensitive(jsonData, "name");
    if (cJSON_IsString(name) && (name->valuestring != NULL))
    {
        printf("Location Name is \"%s\"\n", name->valuestring);
    }

    const cJSON *main = NULL;

	const cJSON *weather = NULL;

	Weather current;

	weather = cJSON_GetObjectItemCaseSensitive(jsonData, "weather");


	const cJSON *weatherItem = NULL;

	weatherItem = cJSON_GetArrayItem(weather, 0);

	if (cJSON_IsObject(weatherItem))
	{

		const cJSON *main = NULL;
		const cJSON *description = NULL;

		main = cJSON_GetObjectItemCaseSensitive(weatherItem, "main");
	//	printf("main weather: %s", main->valuestring);

		description = cJSON_GetObjectItemCaseSensitive(weatherItem, 
				"description");

		strncpy(current.main, main->valuestring, sizeof(current.main));
		strncpy(current.description, description->valuestring,
			sizeof(current.description));

	}

    main = cJSON_GetObjectItemCaseSensitive(jsonData, "main");
    if (cJSON_IsObject(main) && (main->string != NULL))
    {
        const cJSON *temp = NULL;
        const cJSON *feels_like = NULL;
        const cJSON *temp_min = NULL;
        const cJSON *temp_max = NULL;

        temp = cJSON_GetObjectItemCaseSensitive(main,
                        "temp");
        if (cJSON_IsNumber(temp))
        {

             //   printf("temp is \"%f\"\n", temp->valuedouble);
				current.temp = kelvin_to_celsius(temp->valuedouble);
        }

        feels_like = cJSON_GetObjectItemCaseSensitive(main,
                        "feels_like");
        if (cJSON_IsNumber(feels_like))
        {

            //    printf("feels_like is \"%f\"\n", feels_like->valuedouble);
				current.feels_like = 
					kelvin_to_celsius(feels_like->valuedouble);
        }

        temp_min = cJSON_GetObjectItemCaseSensitive(main,
                        "temp_min");
        if (cJSON_IsNumber(temp_min))
        {

             //   printf("temp_min is \"%f\"\n", temp_min->valuedouble);
				current.temp_min = kelvin_to_celsius(temp_min->valuedouble);
        }

        temp_max = cJSON_GetObjectItemCaseSensitive(main,
                        "temp_max");
        if (cJSON_IsNumber(temp_max))
        {

           //     printf("temp_max is \"%f\"\n", temp_max->valuedouble);
				current.temp_max = kelvin_to_celsius(temp_max->valuedouble);
        }

		

	


    }

	const cJSON *sys = NULL;
	sys = cJSON_GetObjectItemCaseSensitive(jsonData, "sys");

	if (cJSON_IsObject(sys) && (sys->string != NULL))
	{

		current.sunrise = cJSON_GetObjectItemCaseSensitive(sys,
				"sunrise")->valueint;
		current.sunset = cJSON_GetObjectItemCaseSensitive(sys,
				"sunset")->valueint;
	
	}
	current.dt = cJSON_GetObjectItemCaseSensitive(jsonData, "dt")->valueint;
	return current;

}

char* format_time(int timevalue)
{

  char* timeBuffer = malloc(12); // big enough to accommodate "hh:mm:ss am"
  time_t ts = timevalue;
  struct tm *timeInfo = localtime(&ts);
  strftime(timeBuffer, sizeof(timeBuffer), "%I:%M%p", timeInfo);
  return timeBuffer;

}

#endif
