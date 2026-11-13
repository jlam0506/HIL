// With 100s of Treatment Duration, you can save over 128 (~ 130 precisely) Treament Data in a 3MB Filesystem. Each 100s Treatment Data is 24KB and its Metadata is 64B
// This means that without syncing with the server, offline storage can last over 4 months for 1 treatment/day or over 1 month for 4 treatments/day

#include "FS.h"
#include <LittleFS.h>
#include <SPIFFS.h>

// Serial Peripheral Interface FAT File System (SPIFFS) Definitions
#define SPIFFS                                  LittleFS
// #define SPIFFS                                  SPIFFS
#define FORMAT_SPIFFS_IF_FAILED                 true

typedef enum verification: uint8_t {
  VERIFICATION_NEBULIZATION_OFF,
  VERIFICATION_NEBULIZATION_ON,
  VERIFICATION_VIBRATION_A,
  VERIFICATION_VIBRATION_B,
  VERIFICATION_VIBRATION_OFF
} verification_t;

typedef struct breath_sample {
  uint32_t timestamp;
  float flow_rate;
  verification_t nebulization;                     // Used for verification test only
  verification_t vibration;                        // Used for verification test only
} breath_sample_t;

typedef struct metadata {
    uint64_t date_and_time;
    float peak_flow;
    float inhaled_volume;
    float flow_score;
    float time_score;
    float inhalation_score;
    uint32_t expected_inhalation_duration;
    uint32_t sum_of_expected_inhalation_duration;
    uint32_t nebulization_duration;
    uint32_t breath_count;
    uint32_t hag_id;
    uint32_t hag_usage;
    bool shared;
} metadata_t;

constexpr char METADATA_DATA_FILE_EXTENSION[] = ".md";
constexpr char TREATMENT_DATA_FILE_EXTENSION[] = ".td";
constexpr float FLOW_RATE_3LPM = 3.0;
constexpr float FLOW_RATE_16LPM = 16.0;
constexpr float FLOW_RATE_18LPM = 18.0;
constexpr float START_OF_NEW_BREATH_DELIMITER_IN_MEMORY = -1000;
static metadata_t metadata {0, 0, 0, 0, 0, 0, 0, 0, 0, 20, 0, 0, false};
constexpr const int MAXIMUM_DATA_SAMPLE = 2000;
static uint32_t sample{MAXIMUM_DATA_SAMPLE};
static breath_sample_t samples[MAXIMUM_DATA_SAMPLE] {{ 0, START_OF_NEW_BREATH_DELIMITER_IN_MEMORY, VERIFICATION_NEBULIZATION_ON, VERIFICATION_VIBRATION_OFF }};

uint64_t get_build_timestamp (void) {
  struct tm tm = {};

  // Parse "__DATE__ __TIME__"
  strptime(__DATE__ " " __TIME__,
            "%b %d %Y %H:%M:%S",
            &tm);

  // Force UTC13
  // setenv("TZ", "UTC0", 1);
  // tzset();

  // Convert to Unix timestamp
  // return (uint64_t)mktime(&tm) - 3600;
  return (uint64_t)mktime(&tm) - 3600;
}

bool save (const breath_sample_t breath_samples[], const uint32_t& size) {
  char file_name[32];
  snprintf(file_name,
         sizeof(file_name),
         "/%" PRIu64 ".td",
         metadata.date_and_time);
  Serial.printf("file_name (TD): %s\n", file_name);
  File file = SPIFFS.open(file_name, FILE_APPEND);

  if (!file) {
    Serial.println("Failed to open file for writing");
    return false;
  }
  
  // TODO: CAUTION! Enable this block of code in the main application
  // Write a file delimiter to indicate the start of a new breath cycle samples storage
  // const breath_sample_t start_of_new_breath = {size+1, START_OF_NEW_BREATH_DELIMITER_IN_MEMORY, VERIFICATION_NEBULIZATION_OFF, VERIFICATION_VIBRATION_OFF};
  // file.write((const uint8_t*)&start_of_new_breath, sizeof(breath_sample_t));

  // Write the actual generated breath samples from the last breath cycle
  size_t bytes_written = file.write((const uint8_t*)breath_samples, size * sizeof(breath_sample_t));
  file.close();

  if (bytes_written != size * sizeof(breath_sample_t)) {
    Serial.println(F("Failed to write all breath samples"));
    return false;
  }

  return true;
}

bool load (const char* file_name, breath_sample_t breath_samples[], const uint32_t& size) {
  File file = SPIFFS.open(file_name, FILE_READ);

  if (!file) {
    return false;
  }

  size_t bytes_read = file.read((uint8_t*)breath_samples, size * sizeof(breath_sample_t));
  file.close();

  return bytes_read == (size * sizeof(breath_sample_t));
}

bool save (const metadata_t& metadata) {
  char file_name[32];
  snprintf(file_name,
         sizeof(file_name),
         "/%" PRIu64 ".md",
         metadata.date_and_time);
  Serial.printf("file_name (MD): %s\n", file_name);
  File file = SPIFFS.open(file_name, FILE_WRITE);
  if (!file) {
    Serial.println(F("Failed to open metadata file for writing"));
    return false;
  }

  size_t bytes_written = file.write((const uint8_t*)&metadata, sizeof(metadata_t));
  file.close();

  if (bytes_written != sizeof(metadata_t)) {
    Serial.println(F("Failed to write metadata"));
    return false;
  }
  return true;
}

bool load (const char* file_name, metadata_t& metadata) {
  File file = SPIFFS.open(file_name, FILE_READ);
  if (!file) {
    return false;
  }

  size_t bytes_read = file.read((uint8_t*)&metadata, sizeof(metadata_t));
  file.close();

  return bytes_read == sizeof(metadata_t);
}

bool get_file_name (const char* file_extension, char* file_name, uint32_t& file_size) {
  constexpr const uint32_t NO_FILE_FOUND = 0;
  file_size = NO_FILE_FOUND;

  File root = SPIFFS.open("/");
  if (!root) {
    Serial.println("- failed to open directory");
    return false;
  }
  if (!root.isDirectory()) {
    Serial.println(" - not a directory");
    return false;
  }

  root.rewindDirectory();
  File file = root.openNextFile();
  while (file) {
    if (file.isDirectory()) {
      Serial.print("  DIR : ");
      Serial.println(file.name());
      // if (levels) {
      //   listDir(fs, file.path(), levels - 1);
      // }
    } else {
      const char* name = file.name();
      if (strstr(name, file_extension) != NULL) {
        Serial.print("  FILE: ");
        Serial.print(name);
        Serial.print("\tSIZE: ");
        Serial.println(file.size());
        strcpy(file_name, name);
        file_size = file.size();
        file.close();
        return true;
      }
    }
    file.close();
    file = root.openNextFile();
  }
  file.close();
  return false;
}

bool compute_treatment_data (void) {
  char file_name[32];
  uint32_t length {0};
  uint32_t flow_rate_above_3lpm {0};
  uint32_t flow_rate_between_3lpm_and_18lpm {0};
  uint32_t sum_of_inhalation_duration {0};

  if (metadata.breath_count == 0) {
    Serial.printf("\nNo Treatment Data found\n");
    return false;
  }
  
  snprintf(file_name, sizeof(file_name), "/%" PRIu64 ".td", metadata.date_and_time);
  File file = SPIFFS.open(file_name, FILE_READ);
  if (!file) {
    return false;
  }
  Serial.printf("\n\n\n\nAll %" PRIu32 " Treatement Data retrieved are displayed below.\nReading from %s with file size: %" PRIu32 "\n\n", metadata.breath_count, file_name, file.size());
  length = file.size()/sizeof(breath_sample_t);
  file.close();
  breath_sample_t *samples = (breath_sample_t *)malloc(length * sizeof(breath_sample_t));
  if (samples == NULL) {
    Serial.println(F("Memory allocation failed"));
    free(samples);
    return false;
  }
  if (!load(file_name, samples, length)) {
    free(samples);
    return false;
  }

  for (uint32_t i = 0, serial_number = 1, breath_count = 0; i < length; i++) {
    if (samples[i].flow_rate == START_OF_NEW_BREATH_DELIMITER_IN_MEMORY) {
      serial_number = 1;
      breath_count++;
      Serial.printf("\n\nBreath Samples %" PRIu32 "\n", breath_count);
      Serial.printf("\nS/No. \t Flow Rate(l/min) \t Timestamp (μS) \t Nebulization \t Vibration\n");
      delay(50);
      continue;
    }

    Serial.printf("[%" PRIu32 "] \t %f \t \t %" PRIu32 " \t \t %s \t \t %s\n", \
      serial_number++, \
      samples[i].flow_rate, \
      samples[i].timestamp, \
      samples[i].nebulization == VERIFICATION_NEBULIZATION_ON ? "ON" : "OFF", \
      samples[i].vibration == VERIFICATION_VIBRATION_A ? "A" : samples[i].vibration == VERIFICATION_VIBRATION_B ? "B" : "OFF");

    sum_of_inhalation_duration += i > 0 ? samples[i].timestamp - samples[i-1].timestamp : 0;
    if (samples[i].flow_rate > FLOW_RATE_3LPM) {
      flow_rate_above_3lpm++;
      if (samples[i].flow_rate < FLOW_RATE_18LPM) {
          flow_rate_between_3lpm_and_18lpm++;
        }
    }

    // 1. Peak Flow
    // Peak Flow = max (∀ samples in the treatment)
    metadata.peak_flow = samples[i].flow_rate > metadata.peak_flow ? samples[i].flow_rate : metadata.peak_flow;
    
    // 2. Inhaled Volume (We could also have Drug Inhaled Volume)
    // Inhaled Volume F(t) = ∫ (Area under the breath curves) dt
    //                     =  ∑∀ ∑ⁿᵢ₌₁ (tᵢ - tᵢ₋₁) * ((fᵢ + fᵢ₋₁)/2)
    if (i > 0) {
      // constexpr float MINUTE_IN_MICROSECONDS = 60 * 1000 * 1000;
      // metadata.inhaled_volume += (((samples[i].timestamp - samples[i-1].timestamp) / MINUTE_IN_MICROSECONDS) * ((samples[i].flow_rate + samples[i-1].flow_rate) / 2.0));
      constexpr float MINUTE_IN_MILLISECONDS = 60 * 1000;
      metadata.inhaled_volume += (((samples[i].timestamp - samples[i-1].timestamp) / MINUTE_IN_MILLISECONDS) * ((samples[i].flow_rate + samples[i-1].flow_rate) / 2.0));
    }
  }

  // 3. Flow Score
  // Flow Score = (number of samples between 3L/min start to 3L/min end) / (number of samples between 3L/min start to 18L/min end) * 100%
  metadata.flow_score = ((float) flow_rate_between_3lpm_and_18lpm / (float) flow_rate_above_3lpm) * 100;

  // 4. Time Score
  // Time Score = ((∑∀ inhalation duration / breath count) / (∑∀ expected inhalation duration / breath count)) * 100%
  // This implies, Time Score =  ⌈100% * (∑∀ inhalation duration / ∑∀ expected inhalation duration)⌉
  metadata.time_score = ((float) sum_of_inhalation_duration / (float) metadata.sum_of_expected_inhalation_duration) * 100.0;
  metadata.time_score = metadata.time_score > 100 ? 100 : metadata.time_score < 0 ? 0 : metadata.time_score;

  // 5. Inhalation Score
  // Inhalation Score = (Flow Score + Flow Score + Time Score) / 3;
  metadata.inhalation_score = (metadata.flow_score + metadata.flow_score + metadata.time_score) / 3.0;

  // Store, display and reset all metadata calculated results
  save(metadata);
  Serial.printf("\n\npeak_flow: %f\ninhaled_volume: %f\nflow_score: %f\ntime_score: %f\ninhalation_score: %f\nexpected_inhalation_duration: %" PRIu32 "\nnebulization_duration: %" PRIu32 "\nbreath_count: %" PRIu32 "\nhag_id: %" PRIu32 "\nhag_usage: %" PRIu32 "\nfile_shared: %d\n\n", \
      metadata.peak_flow, metadata.inhaled_volume, metadata.flow_score, metadata.time_score, metadata.inhalation_score, \
      metadata.expected_inhalation_duration, metadata.nebulization_duration, metadata.breath_count, metadata.hag_id, metadata.hag_usage, metadata.shared);
  // metadata = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, false};
  free(samples);
  return true;
}

bool share_treatment_data (void) {
  metadata_t md {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, false};
  char file_name[32];
  uint32_t length {0};

  if (!get_file_name(METADATA_DATA_FILE_EXTENSION, file_name, length)) {
    //BluetoothLowEnergy.share_treatment_data(NULL, 0, 0, "end");
    return false;
  }

  if (length == 0) {
    //BluetoothLowEnergy.share_treatment_data(NULL, 0, 0, "end");
    return true;
  }
  
  // Change file_name from xxx.md to /xxx.md
  if (!load(String("/" + String(file_name)).c_str(), md)) return false;

  if (md.breath_count > 0) {
    // Change file_name from /xxx.md to /xxx.td
    snprintf(file_name, sizeof(file_name), "/%" PRIu64 ".td", md.date_and_time);
    File file = SPIFFS.open(file_name, FILE_READ);
    if (!file) {
      return false;
    }

    length = file.size()/sizeof(breath_sample_t);
    file.close();
    breath_sample_t *samples = (breath_sample_t *)malloc(length * sizeof(breath_sample_t));
    if (samples == NULL) {
      Serial.println(F("Memory allocation failed"));
      free(samples);
      return false;
    }
    if (!load(file_name, samples, length)) {
      free(samples);
      return false;
    }

    Serial.printf("\n\nTreatement Data Filename: %s\n", file_name);
    
    for (uint32_t i = 0, serial_number = 1, breath_count = 0; i < length; i++) {
      if (samples[i].flow_rate == START_OF_NEW_BREATH_DELIMITER_IN_MEMORY) {
        serial_number = 1;
        breath_count++;
        // Please note that the timestamp, just below, holds the number of samples as stored by save(samples, sample);
        //md.shared = BluetoothLowEnergy.share_treatment_data(samples, samples[i].timestamp, breath_count, breath_count == md.breath_count ? "end" : "");
        Serial.printf("\n\nBreath Samples %" PRIu32 "\n", breath_count);
        Serial.printf("\nS/No. \t Flow Rate(l/min) \t Timestamp (μS) \t Nebulization \t Vibration\n");
        delay(50);
        continue;
      }
      Serial.printf("[%" PRIu32 "] \t %f \t \t %" PRIu32 " \t \t %s \t \t %s\n", \
      serial_number++, \
      samples[i].flow_rate, \
      samples[i].timestamp, \
      samples[i].nebulization == VERIFICATION_NEBULIZATION_ON ? "ON" : "OFF", \
      samples[i].vibration == VERIFICATION_VIBRATION_A ? "A" : samples[i].vibration == VERIFICATION_VIBRATION_B ? "B" : "OFF");
    }
    free(samples);
  }
  else {
      Serial.printf("\n[Share] No Treatment Data found in the Flash\n");
      //BluetoothLowEnergy.share_treatment_data(NULL, 0, 0, "end");
  }
  return true;
}

void setup() {
  // Initialize the Universal Asynchronous Receive-Transmit (UART) Communication Interface:
  Serial.begin(DEBUG_BAUD_RATE);
  delay(DELAY_ON_BOOT);
  Serial.printf("KOLIBRI HARDWARE REVISION: %s\n", KOLIBRI_HARDWARE_REVISION);

  // Initialize the SPI Flash File System (SPIFFS) Memory
  if(!SPIFFS.begin(FORMAT_SPIFFS_IF_FAILED)) {
    Serial.println("[SPIFFS] Initialization Failed!");
  }
  else {
    Serial.println("[SPIFFS] Initialization Successful!");
  }

  metadata.date_and_time = get_build_timestamp();
  for (uint32_t i = 0; i < sample; i++) {
    if ((i%100 == 0) || (i == 0)) {
      samples[i].timestamp = 100;
      samples[i].flow_rate = START_OF_NEW_BREATH_DELIMITER_IN_MEMORY;
      samples[i].nebulization = VERIFICATION_NEBULIZATION_OFF;
      samples[i].vibration = VERIFICATION_VIBRATION_OFF;
      continue;
    }
    static uint32_t gap = 0;
    // samples[i].timestamp = metadata.date_and_time + gap + random(1,50);
    samples[i].timestamp = gap + random(1,5);
    samples[i].flow_rate = (float)random(1000, 20000) / 1000.0;;
    samples[i].nebulization = ((samples[i].flow_rate > FLOW_RATE_3LPM) && (samples[i].flow_rate <= FLOW_RATE_18LPM)) ? VERIFICATION_NEBULIZATION_ON : VERIFICATION_NEBULIZATION_OFF;
    samples[i].vibration = ((samples[i].flow_rate > FLOW_RATE_3LPM) && (samples[i].flow_rate <= FLOW_RATE_16LPM)) ? VERIFICATION_VIBRATION_A : ((samples[i].flow_rate > FLOW_RATE_16LPM) && (samples[i].flow_rate <= FLOW_RATE_18LPM)) ? VERIFICATION_VIBRATION_B : VERIFICATION_VIBRATION_OFF;
    gap += 50;
  }

  save(samples, sample);
  save(metadata);

  compute_treatment_data();
  delay(3000);
  share_treatment_data();  
}

void loop() {
  // put your main code here, to run repeatedly:

}
