/**
 * @file model.h
 * @author Alessandro Ferrante (github@alessandroferrante.net)
 * @brief This file contains a pre-trained model.
 * The model is designed to:
 * 1. Predict carbon monoxide (CO) ppm values ​​using temperature and humidity as features.
 * 2. Perform classification of ppm, temperature and humidity values.
 *
 * The model is serialized in binary format and stored in the `model_data` array.
 * The length of the model is specified by the `model_data_len` variable.
 * @version 0.1
 * 
 * @copyright Copyright (c) 2025
 * 
 */

#ifdef __has_attribute
#define HAVE_ATTRIBUTE(x) __has_attribute(x)
#else
#define HAVE_ATTRIBUTE(x) 0
#endif
#if HAVE_ATTRIBUTE(aligned) || (defined(__GNUC__) && !defined(__clang__))
#define DATA_ALIGN_ATTRIBUTE __attribute__((aligned(4)))
#else
#define DATA_ALIGN_ATTRIBUTE
#endif

const unsigned char model_data[] DATA_ALIGN_ATTRIBUTE = {
	0x1c, 0x00, 0x00, 0x00, 0x54, 0x46, 0x4c, 0x33, 0x14, 0x00, 0x20, 0x00, 
	0x1c, 0x00, 0x18, 0x00, 0x14, 0x00, 0x10, 0x00, 0x0c, 0x00, 0x00, 0x00, 
	0x08, 0x00, 0x04, 0x00, 0x14, 0x00, 0x00, 0x00, 0x1c, 0x00, 0x00, 0x00, 
	0x9c, 0x00, 0x00, 0x00, 0xf4, 0x00, 0x00, 0x00, 0xd0, 0x01, 0x00, 0x00, 
	0xe0, 0x01, 0x00, 0x00, 0x18, 0x04, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 
	0x01, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0a, 0x00, 
	0x10, 0x00, 0x0c, 0x00, 0x08, 0x00, 0x04, 0x00, 0x0a, 0x00, 0x00, 0x00, 
	0x0c, 0x00, 0x00, 0x00, 0x1c, 0x00, 0x00, 0x00, 0x3c, 0x00, 0x00, 0x00, 
	0x0f, 0x00, 0x00, 0x00, 0x73, 0x65, 0x72, 0x76, 0x69, 0x6e, 0x67, 0x5f, 
	0x64, 0x65, 0x66, 0x61, 0x75, 0x6c, 0x74, 0x00, 0x01, 0x00, 0x00, 0x00, 
	0x04, 0x00, 0x00, 0x00, 0x8c, 0xff, 0xff, 0xff, 0x03, 0x00, 0x00, 0x00, 
	0x04, 0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00, 0x6f, 0x75, 0x74, 0x70, 
	0x75, 0x74, 0x5f, 0x30, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 
	0x04, 0x00, 0x00, 0x00, 0xc2, 0xfe, 0xff, 0xff, 0x04, 0x00, 0x00, 0x00, 
	0x10, 0x00, 0x00, 0x00, 0x6b, 0x65, 0x72, 0x61, 0x73, 0x5f, 0x74, 0x65, 
	0x6e, 0x73, 0x6f, 0x72, 0x5f, 0x34, 0x38, 0x36, 0x00, 0x00, 0x00, 0x00, 
	0x02, 0x00, 0x00, 0x00, 0x34, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 
	0xdc, 0xff, 0xff, 0xff, 0x06, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 
	0x13, 0x00, 0x00, 0x00, 0x43, 0x4f, 0x4e, 0x56, 0x45, 0x52, 0x53, 0x49, 
	0x4f, 0x4e, 0x5f, 0x4d, 0x45, 0x54, 0x41, 0x44, 0x41, 0x54, 0x41, 0x00, 
	0x08, 0x00, 0x0c, 0x00, 0x08, 0x00, 0x04, 0x00, 0x08, 0x00, 0x00, 0x00, 
	0x05, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x13, 0x00, 0x00, 0x00, 
	0x6d, 0x69, 0x6e, 0x5f, 0x72, 0x75, 0x6e, 0x74, 0x69, 0x6d, 0x65, 0x5f, 
	0x76, 0x65, 0x72, 0x73, 0x69, 0x6f, 0x6e, 0x00, 0x07, 0x00, 0x00, 0x00, 
	0xd8, 0x00, 0x00, 0x00, 0xd0, 0x00, 0x00, 0x00, 0xbc, 0x00, 0x00, 0x00, 
	0x9c, 0x00, 0x00, 0x00, 0x94, 0x00, 0x00, 0x00, 0x74, 0x00, 0x00, 0x00, 
	0x04, 0x00, 0x00, 0x00, 0x5e, 0xff, 0xff, 0xff, 0x04, 0x00, 0x00, 0x00, 
	0x60, 0x00, 0x00, 0x00, 0x0c, 0x00, 0x00, 0x00, 0x08, 0x00, 0x0e, 0x00, 
	0x08, 0x00, 0x04, 0x00, 0x08, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 
	0x28, 0x00, 0x00, 0x00, 0x00, 0x00, 0x06, 0x00, 0x08, 0x00, 0x04, 0x00, 
	0x06, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 
	0xea, 0x03, 0x00, 0x00, 0x0c, 0x00, 0x18, 0x00, 0x14, 0x00, 0x10, 0x00, 
	0x0c, 0x00, 0x04, 0x00, 0x0c, 0x00, 0x00, 0x00, 0xb5, 0x63, 0x30, 0x69, 
	0xf2, 0xec, 0x19, 0x18, 0x02, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 
	0x04, 0x00, 0x00, 0x00, 0x06, 0x00, 0x00, 0x00, 0x32, 0x2e, 0x31, 0x38, 
	0x2e, 0x30, 0x00, 0x00, 0xca, 0xff, 0xff, 0xff, 0x04, 0x00, 0x00, 0x00, 
	0x10, 0x00, 0x00, 0x00, 0x31, 0x2e, 0x35, 0x2e, 0x30, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xb4, 0xfd, 0xff, 0xff, 
	0xea, 0xff, 0xff, 0xff, 0x04, 0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00, 
	0xca, 0xa8, 0x43, 0x3e, 0xcc, 0xf8, 0xa8, 0x3d, 0x00, 0x00, 0x06, 0x00, 
	0x08, 0x00, 0x04, 0x00, 0x06, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 
	0x04, 0x00, 0x00, 0x00, 0x0e, 0xdb, 0x99, 0xbc, 0xe4, 0xfd, 0xff, 0xff, 
	0xe8, 0xfd, 0xff, 0xff, 0x0f, 0x00, 0x00, 0x00, 0x4d, 0x4c, 0x49, 0x52, 
	0x20, 0x43, 0x6f, 0x6e, 0x76, 0x65, 0x72, 0x74, 0x65, 0x64, 0x2e, 0x00, 
	0x01, 0x00, 0x00, 0x00, 0x14, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0e, 0x00, 
	0x18, 0x00, 0x14, 0x00, 0x10, 0x00, 0x0c, 0x00, 0x08, 0x00, 0x04, 0x00, 
	0x0e, 0x00, 0x00, 0x00, 0x14, 0x00, 0x00, 0x00, 0x1c, 0x00, 0x00, 0x00, 
	0x60, 0x00, 0x00, 0x00, 0x64, 0x00, 0x00, 0x00, 0x68, 0x00, 0x00, 0x00, 
	0x04, 0x00, 0x00, 0x00, 0x6d, 0x61, 0x69, 0x6e, 0x00, 0x00, 0x00, 0x00, 
	0x01, 0x00, 0x00, 0x00, 0x14, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0e, 0x00, 
	0x14, 0x00, 0x00, 0x00, 0x10, 0x00, 0x0c, 0x00, 0x0b, 0x00, 0x04, 0x00, 
	0x0e, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x08, 
	0x0c, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x68, 0xfe, 0xff, 0xff, 
	0x01, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 
	0x01, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x40, 0x01, 0x00, 0x00, 
	0xcc, 0x00, 0x00, 0x00, 0x60, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 
	0xe6, 0xfe, 0xff, 0xff, 0x00, 0x00, 0x00, 0x01, 0x14, 0x00, 0x00, 0x00, 
	0x1c, 0x00, 0x00, 0x00, 0x1c, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 
	0x34, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 
	0x01, 0x00, 0x00, 0x00, 0xd0, 0xfe, 0xff, 0xff, 0x1b, 0x00, 0x00, 0x00, 
	0x53, 0x74, 0x61, 0x74, 0x65, 0x66, 0x75, 0x6c, 0x50, 0x61, 0x72, 0x74, 
	0x69, 0x74, 0x69, 0x6f, 0x6e, 0x65, 0x64, 0x43, 0x61, 0x6c, 0x6c, 0x5f, 
	0x31, 0x3a, 0x30, 0x00, 0x02, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 
	0x01, 0x00, 0x00, 0x00, 0xae, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x01, 
	0x10, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 
	0x30, 0x00, 0x00, 0x00, 0x18, 0xff, 0xff, 0xff, 0x20, 0x00, 0x00, 0x00, 
	0x73, 0x65, 0x71, 0x75, 0x65, 0x6e, 0x74, 0x69, 0x61, 0x6c, 0x5f, 0x31, 
	0x5f, 0x31, 0x2f, 0x64, 0x65, 0x6e, 0x73, 0x65, 0x5f, 0x34, 0x39, 0x5f, 
	0x31, 0x2f, 0x4d, 0x61, 0x74, 0x4d, 0x75, 0x6c, 0x00, 0x00, 0x00, 0x00, 
	0x02, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x16, 0x00, 0x18, 0x00, 0x14, 0x00, 0x00, 0x00, 0x10, 0x00, 
	0x0c, 0x00, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x07, 0x00, 
	0x16, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x10, 0x00, 0x00, 0x00, 
	0x10, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x3c, 0x00, 0x00, 0x00, 
	0x80, 0xff, 0xff, 0xff, 0x2c, 0x00, 0x00, 0x00, 0x73, 0x65, 0x71, 0x75, 
	0x65, 0x6e, 0x74, 0x69, 0x61, 0x6c, 0x5f, 0x31, 0x5f, 0x31, 0x2f, 0x64, 
	0x65, 0x6e, 0x73, 0x65, 0x5f, 0x34, 0x39, 0x5f, 0x31, 0x2f, 0x41, 0x64, 
	0x64, 0x2f, 0x52, 0x65, 0x61, 0x64, 0x56, 0x61, 0x72, 0x69, 0x61, 0x62, 
	0x6c, 0x65, 0x4f, 0x70, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 
	0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x16, 0x00, 0x1c, 0x00, 0x18, 0x00, 
	0x00, 0x00, 0x14, 0x00, 0x10, 0x00, 0x0c, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x08, 0x00, 0x07, 0x00, 0x16, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 
	0x14, 0x00, 0x00, 0x00, 0x20, 0x00, 0x00, 0x00, 0x20, 0x00, 0x00, 0x00, 
	0x01, 0x00, 0x00, 0x00, 0x40, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 
	0xff, 0xff, 0xff, 0xff, 0x02, 0x00, 0x00, 0x00, 0x04, 0x00, 0x04, 0x00, 
	0x04, 0x00, 0x00, 0x00, 0x22, 0x00, 0x00, 0x00, 0x73, 0x65, 0x72, 0x76, 
	0x69, 0x6e, 0x67, 0x5f, 0x64, 0x65, 0x66, 0x61, 0x75, 0x6c, 0x74, 0x5f, 
	0x6b, 0x65, 0x72, 0x61, 0x73, 0x5f, 0x74, 0x65, 0x6e, 0x73, 0x6f, 0x72, 
	0x5f, 0x34, 0x38, 0x36, 0x3a, 0x30, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 
	0x01, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 
	0x10, 0x00, 0x00, 0x00, 0x0c, 0x00, 0x0c, 0x00, 0x0b, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x04, 0x00, 0x0c, 0x00, 0x00, 0x00, 0x09, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x09
};
const int model_data_len = 1132;