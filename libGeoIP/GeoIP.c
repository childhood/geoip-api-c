/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 2; tab-width: 2 -*- */
/* GeoIP.c
 *
 * Copyright (C) 2002 MaxMind.com.  All Rights Reserved.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "GeoIP.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <assert.h>
#include <sys/types.h> /* for fstat */
#include <sys/stat.h>	/* for fstat */

const int COUNTRY_BEGIN = 16776960;
const int STATE_BEGIN   = 16700000;
const int STRUCTURE_INFO_MAX_SIZE = 20;
const int DATABASE_INFO_MAX_SIZE = 100;
const int MAX_ORG_RECORD_LENGTH = 300;

#define CHECK_ERR(err, msg) { \
		if (err != Z_OK) { \
				fprintf(stderr, "%s error: %d\n", msg, err); \
				exit(1); \
		} \
}

const char GeoIP_country_code[247][3] = { "--","AP","EU","AD","AE","AF","AG","AI","AL","AM","AN","AO","AQ","AR","AS","AT","AU","AW","AZ","BA","BB","BD","BE","BF","BG","BH","BI","BJ","BM","BN","BO","BR","BS","BT","BV","BW","BY","BZ","CA","CC","CD","CF","CG","CH","CI","CK","CL","CM","CN","CO","CR","CU","CV","CX","CY","CZ","DE","DJ","DK","DM","DO","DZ","EC","EE","EG","EH","ER","ES","ET","FI","FJ","FK","FM","FO","FR","FX","GA","GB","GD","GE","GF","GH","GI","GL","GM","GN","GP","GQ","GR","GS","GT","GU","GW","GY","HK","HM","HN","HR","HT","HU","ID","IE","IL","IN","IO","IQ","IR","IS","IT","JM","JO","JP","KE","KG","KH","KI","KM","KN","KP","KR","KW","KY","KZ","LA","LB","LC","LI","LK","LR","LS","LT","LU","LV","LY","MA","MC","MD","MG","MH","MK","ML","MM","MN","MO","MP","MQ","MR","MS","MT","MU","MV","MW","MX","MY","MZ","NA","NC","NE","NF","NG","NI","NL","NO","NP","NR","NU","NZ","OM","PA","PE","PF","PG","PH","PK","PL","PM","PN","PR","PS","PT","PW","PY","QA","RE","RO","RU","RW","SA","SB","SC","SD","SE","SG","SH","SI","SJ","SK","SL","SM","SN","SO","SR","ST","SV","SY","SZ","TC","TD","TF","TG","TH","TJ","TK","TM","TN","TO","TP","TR","TT","TV","TW","TZ","UA","UG","UM","US","UY","UZ","VA","VC","VE","VG","VI","VN","VU","WF","WS","YE","YT","YU","ZA","ZM","ZR","ZW","A1","A2","O1"};

const char GeoIP_country_code3[247][4] = { "--","AP","EU","AND","ARE","AFG","ATG","AIA","ALB","ARM","ANT","AGO","AQ","ARG","ASM","AUT","AUS","ABW","AZE","BIH","BRB","BGD","BEL","BFA","BGR","BHR","BDI","BEN","BMU","BRN","BOL","BRA","BHS","BTN","BV","BWA","BLR","BLZ","CAN","CC","COD","CAF","COG","CHE","CIV","COK","CHL","CMR","CHN","COL","CRI","CUB","CPV","CX","CYP","CZE","DEU","DJI","DNK","DMA","DOM","DZA","ECU","EST","EGY","ESH","ERI","ESP","ETH","FIN","FJI","FLK","FSM","FRO","FRA","FX","GAB","GBR","GRD","GEO","GUF","GHA","GIB","GRL","GMB","GIN","GLP","GNQ","GRC","GS","GTM","GUM","GNB","GUY","HKG","HM","HND","HRV","HTI","HUN","IDN","IRL","ISR","IND","IO","IRQ","IRN","ISL","ITA","JAM","JOR","JPN","KEN","KGZ","KHM","KIR","COM","KNA","PRK","KOR","KWT","CYM","KAZ","LAO","LBN","LCA","LIE","LKA","LBR","LSO","LTU","LUX","LVA","LBY","MAR","MCO","MDA","MDG","MHL","MKD","MLI","MMR","MNG","MAC","MNP","MTQ","MRT","MSR","MLT","MUS","MDV","MWI","MEX","MYS","MOZ","NAM","NCL","NER","NFK","NGA","NIC","NLD","NOR","NPL","NRU","NIU","NZL","OMN","PAN","PER","PYF","PNG","PHL","PAK","POL","SPM","PCN","PRI","PSE","PRT","PLW","PRY","QAT","REU","ROU","RUS","RWA","SAU","SLB","SYC","SDN","SWE","SGP","SHN","SVN","SJM","SVK","SLE","SMR","SEN","SOM","SUR","STP","SLV","SYR","SWZ","TCA","TCD","TF","TGO","THA","TJK","TKL","TLS","TKM","TUN","TON","TUR","TTO","TUV","TWN","TZA","UKR","UGA","UM","USA","URY","UZB","VAT","VCT","VEN","VGB","VIR","VNM","VUT","WLF","WSM","YEM","YT","YUG","ZAF","ZMB","ZR","ZWE","A1","A2","O1"};

const char * GeoIP_country_name[247] = {"N/A","Asia/Pacific Region","Europe","Andorra","United Arab Emirates","Afghanistan","Antigua and Barbuda","Anguilla","Albania","Armenia","Netherlands Antilles","Angola","Antarctica","Argentina","American Samoa","Austria","Australia","Aruba","Azerbaijan","Bosnia and Herzegovina","Barbados","Bangladesh","Belgium","Burkina Faso","Bulgaria","Bahrain","Burundi","Benin","Bermuda","Brunei Darussalam","Bolivia","Brazil","Bahamas","Bhutan","Bouvet Island","Botswana","Belarus","Belize","Canada","Cocos (Keeling) Islands","Congo, The Democratic Republic of the","Central African Republic","Congo","Switzerland","Cote D'Ivoire","Cook Islands","Chile","Cameroon","China","Colombia","Costa Rica","Cuba","Cape Verde","Christmas Island","Cyprus","Czech Republic","Germany","Djibouti","Denmark","Dominica","Dominican Republic","Algeria","Ecuador","Estonia","Egypt","Western Sahara","Eritrea","Spain","Ethiopia","Finland","Fiji","Falkland Islands (Malvinas)","Micronesia, Federated States of","Faroe Islands","France","France, Metropolitan","Gabon","United Kingdom","Grenada","Georgia","French Guiana","Ghana","Gibraltar","Greenland","Gambia","Guinea","Guadeloupe","Equatorial Guinea","Greece","South Georgia and the South Sandwich Islands","Guatemala","Guam","Guinea-Bissau","Guyana","Hong Kong","Heard Island and McDonald Islands","Honduras","Croatia","Haiti","Hungary","Indonesia","Ireland","Israel","India","British Indian Ocean Territory","Iraq","Iran, Islamic Republic of","Iceland","Italy","Jamaica","Jordan","Japan","Kenya","Kyrgyzstan","Cambodia","Kiribati","Comoros","Saint Kitts and Nevis",
"Korea, Democratic People's Republic of","Korea, Republic of","Kuwait","Cayman Islands","Kazakhstan","Lao People's Democratic Republic","Lebanon","Saint Lucia","Liechtenstein","Sri Lanka","Liberia","Lesotho","Lithuania","Luxembourg","Latvia","Libyan Arab Jamahiriya","Morocco","Monaco","Moldova, Republic of","Madagascar","Marshall Islands","Macedonia, the Former Yugoslav Republic of","Mali","Myanmar","Mongolia","Macao","Northern Mariana Islands","Martinique","Mauritania","Montserrat","Malta","Mauritius","Maldives","Malawi","Mexico","Malaysia","Mozambique","Namibia","New Caledonia","Niger","Norfolk Island","Nigeria","Nicaragua","Netherlands","Norway","Nepal","Nauru","Niue","New Zealand","Oman","Panama","Peru","French Polynesia","Papua New Guinea","Philippines","Pakistan","Poland","Saint Pierre and Miquelon","Pitcairn","Puerto Rico","Palestinian Territory, Occupied","Portugal","Palau","Paraguay","Qatar","Reunion","Romania","Russian Federation","Rwanda","Saudi Arabia","Solomon Islands","Seychelles","Sudan","Sweden","Singapore","Saint Helena","Slovenia","Svalbard and Jan Mayen","Slovakia","Sierra Leone","San Marino","Senegal","Somalia","Suriname","Sao Tome and Principe","El Salvador","Syrian Arab Republic","Swaziland","Turks and Caicos Islands","Chad","French Southern Territories","Togo","Thailand","Tajikistan","Tokelau","Turkmenistan","Tunisia","Tonga","East Timor","Turkey","Trinidad and Tobago","Tuvalu","Taiwan","Tanzania, United Republic of","Ukraine","Uganda","United States Minor Outlying Islands","United States","Uruguay","Uzbekistan","Holy See (Vatican City State)","Saint Vincent and the Grenadines","Venezuela","Virgin Islands, British","Virgin Islands, U.S.","Vietnam","Vanuatu","Wallis and Futuna","Samoa","Yemen","Mayotte","Yugoslavia","South Africa","Zambia","Zaire","Zimbabwe",
"Anonymous Proxy","Satellite Provider","Other"};

const char *GeoIPDBFileName = GEOIPDATADIR "/GeoIP.dat";

int _check_mtime(GeoIP *gi) {
	struct stat buf;

	if (gi->flags & GEOIP_CHECK_CACHE) {
		if (fstat(fileno(gi->GeoIPDatabase), &buf) != -1) {
			if (buf.st_mtime > gi->mtime) {
				/* GeoIP Database file updated, reload database into memory cache */
				if (realloc(gi->cache, buf.st_size) != NULL) {
					if (fread(gi->cache, sizeof(unsigned char), buf.st_size, gi->GeoIPDatabase) != (size_t) buf.st_size) {
						fprintf(stderr,"Error reading file %s\n",gi->file_path);
						return -1;
					}
					gi->mtime = buf.st_mtime;
				}
			}
		}
	}
	return 0;
}

void _setup_segments(GeoIP * gi) {
	int i, j;
	unsigned char delim[3];
	unsigned char buf[SEGMENT_RECORD_LENGTH];

	/* default to GeoIP Country Edition */
	gi->databaseType = GEOIP_COUNTRY_EDITION;
	gi->record_length = STANDARD_RECORD_LENGTH;
	fseek(gi->GeoIPDatabase, -3l, SEEK_END);
	for (i = 0; i < STRUCTURE_INFO_MAX_SIZE; i++) {
		fread(delim, 1, 3, gi->GeoIPDatabase);
		if (delim[0] == 255 && delim[1] == 255 && delim[2] == 255) {
			fread(&gi->databaseType, 1, 1, gi->GeoIPDatabase);
			if (gi->databaseType == GEOIP_REGION_EDITION) {
				/* Region Edition */
				gi->databaseSegments = malloc(sizeof(int));
				gi->databaseSegments[0] = STATE_BEGIN;
			} else if (gi->databaseType == GEOIP_CITY_EDITION ||
								 gi->databaseType == GEOIP_ORG_EDITION) {
				/* City/Org Editions have two segments, read offset of second segment */
				gi->databaseSegments = malloc(sizeof(int));
				gi->databaseSegments[0] = 0;
				fread(buf, SEGMENT_RECORD_LENGTH, 1, gi->GeoIPDatabase);
				for (j = 0; j < SEGMENT_RECORD_LENGTH; j++) {
					gi->databaseSegments[0] += (buf[j] << (j * 8));
				}
				gi->record_length = ORG_RECORD_LENGTH;
			}
			break;
		} else {
			fseek(gi->GeoIPDatabase, -4l, SEEK_CUR);
		}
	}
	if (gi->databaseType == GEOIP_COUNTRY_EDITION) {
		gi->databaseSegments = malloc(sizeof(int));
		gi->databaseSegments[0] = COUNTRY_BEGIN;
	}
}

unsigned int _seek_country (GeoIP *gi, unsigned long ipnum) {
	int i, j, depth;
	unsigned int x[2];
	unsigned char buf[2][MAX_RECORD_LENGTH];
	unsigned char *cache_buf = NULL;
	unsigned int offset = 0;

	_check_mtime(gi);
	for (depth = 31; depth >= 0; depth--) {
		if (gi->cache == NULL) {
			fseek(gi->GeoIPDatabase, (long)gi->record_length * 2 * offset, SEEK_SET);
			fread(buf,gi->record_length,2,gi->GeoIPDatabase);
		} else {
			cache_buf = gi->cache + (long)gi->record_length * 2 *offset;
		}
		for (i = 0; i < 2; i++) {
			x[i] = 0;
			for (j = 0; j < gi->record_length; j++) {
				if (gi->cache == NULL) {
					x[i] += (buf[i][j] << (j * 8));
				} else {
					x[i] += (cache_buf[i*gi->record_length + j] << (j * 8));
				}
			}
		}

		if (ipnum & (1 << depth)) {
			if (x[1] >= gi->databaseSegments[0]) {
				return x[1];
			}
			offset = x[1];
		} else {
			if (x[0] >= gi->databaseSegments[0]) {
				return x[0];
			}
			offset = x[0];
		}
	}

	/* shouldn't reach here */
	fprintf(stderr,"Error Traversing Database for ipnum = %lu - Perhaps database is corrupt?\n",ipnum);
	return 0;
}

unsigned long _addr_to_num (const char *addr) {
	int i;
	char tok[4];
	int octet;
	int j = 0, k = 0;
	unsigned long ipnum = 0;
	char c = 0;

	for (i=0; i<4; i++) {
		for (;;) {
			c = addr[k++];
			if (c == '.' || c == '\0') {
				tok[j] = '\0';
				octet = atoi(tok);
				if (octet > 255)
					return 0;
				ipnum += (octet << ((3-i)*8));
				j = 0;
				break;
			} else if (c >= '0' && c<= '9') {
				if (j > 2) {
					return 0;
				}
				tok[j++] = c;
			} else {
				return 0;
			}
		}
		if(c == '\0' && i<3) {
			return 0;
		}
	}
	return ipnum;
}

unsigned long _h_addr_to_num (unsigned char *addr) {
	int i;
	unsigned long ipnum = 0;
	unsigned int octet;

	for (i=0; i<4; i++) {
		octet = addr[i];
		ipnum += (octet << ((3-i)*8));
	}
	
	return ipnum;
}

GeoIP* GeoIP_new (int flags) {
	GeoIP * gi;
	gi = GeoIP_open (GeoIPDBFileName, flags);
	return gi;
}

GeoIP* GeoIP_open (const char * filename, int flags) {
	struct stat buf;

	GeoIP *gi = (GeoIP *)malloc(sizeof(GeoIP));
	if (gi == NULL)
		return NULL;
	gi->file_path = malloc(sizeof(char) * (strlen(filename)+1));
	if (gi->file_path == NULL)
		return NULL;
	strcpy(gi->file_path, filename);
	gi->GeoIPDatabase = fopen(filename,"rb");
	if (gi->GeoIPDatabase == NULL) {
		fprintf(stderr,"Error Opening file %s\n",filename);
		free(gi->file_path);
		free(gi);
		return NULL;
	} else {
		if (flags & GEOIP_MEMORY_CACHE) {
			if (fstat(fileno(gi->GeoIPDatabase), &buf) == -1) {
				fprintf(stderr,"Error stating file %s\n",filename);
				free(gi);
				return NULL;
			}
			gi->mtime = buf.st_mtime;
			gi->cache = (unsigned char *) malloc(sizeof(unsigned char) * buf.st_size);
			if (gi->cache != NULL) {
				if (fread(gi->cache, sizeof(unsigned char), buf.st_size, gi->GeoIPDatabase) != (size_t) buf.st_size) {
					fprintf(stderr,"Error reading file %s\n",filename);
					free(gi->cache);
					free(gi);
					return NULL;
				}
			}
		} else {
			gi->cache = NULL;
		}
		gi->flags = flags;
		_setup_segments(gi);
		return gi;
	}
}

void GeoIP_delete (GeoIP *gi) {
	if (gi->GeoIPDatabase != NULL)
		fclose(gi->GeoIPDatabase);
	if (gi->cache != NULL)
		free(gi->cache);
	if (gi->file_path != NULL)
		free(gi->file_path);
	if (gi->databaseSegments != NULL)
		free(gi->databaseSegments);
	free(gi);
}

const char *GeoIP_country_code_by_name (GeoIP* gi, const char *name) {
	int country_id;
	country_id = GeoIP_country_id_by_name(gi, name);
	return (country_id > 0) ? GeoIP_country_code[country_id] : NULL;
}

const char *GeoIP_country_code3_by_name (GeoIP* gi, const char *name) {
	int country_id;
	country_id = GeoIP_country_id_by_name(gi, name);
	return (country_id > 0) ? GeoIP_country_code3[country_id] : NULL;
}

const char *GeoIP_country_name_by_name (GeoIP* gi, const char *name) {
	int country_id;
	country_id = GeoIP_country_id_by_name(gi, name);
	return (country_id > 0) ? GeoIP_country_name[country_id] : NULL;
}

int GeoIP_country_id_by_name (GeoIP* gi, const char *name) {
	unsigned long ipnum;
	int ret;
	struct hostent * host;
	if (name == NULL) {
		return 0;
	}
	ipnum = _addr_to_num(name);
	if (ipnum == 0) {
		host = gethostbyname(name);
		if (host == NULL) {
			return 0;
		}
		ipnum = _h_addr_to_num((unsigned char *) host->h_addr_list[0]);
	}
	ret = _seek_country(gi, ipnum) - COUNTRY_BEGIN;
	return ret;
}

const char *GeoIP_country_code_by_addr (GeoIP* gi, const char *addr) {
	int country_id;
	country_id = GeoIP_country_id_by_addr(gi, addr);
	return (country_id > 0) ? GeoIP_country_code[country_id] : NULL;
}

const char *GeoIP_country_code3_by_addr (GeoIP* gi, const char *addr) {
	int country_id;
	country_id = GeoIP_country_id_by_addr(gi, addr);
	return (country_id > 0) ? GeoIP_country_code3[country_id] : NULL;
	return GeoIP_country_code3[country_id];
}

const char *GeoIP_country_name_by_addr (GeoIP* gi, const char *addr) {
	int country_id;
	country_id = GeoIP_country_id_by_addr(gi, addr);
	return (country_id > 0) ? GeoIP_country_name[country_id] : NULL;
	return GeoIP_country_name[country_id];
}

int GeoIP_country_id_by_addr (GeoIP* gi, const char *addr) {
	unsigned long ipnum;
	int ret;
	if (addr == NULL) {
		return 0;
	}
	ipnum = _addr_to_num(addr);
	ret = _seek_country(gi, ipnum) - COUNTRY_BEGIN;
	return ret;
}

char *GeoIP_database_info (GeoIP* gi) {
	int i;
	unsigned char buf[3];
	char *retval;
	int hasStructureInfo = 0;

	if(gi == NULL)
		return NULL;

	_check_mtime(gi);
	fseek(gi->GeoIPDatabase, -3l, SEEK_END);

	/* first get past the database structure information */
	for (i = 0; i < STRUCTURE_INFO_MAX_SIZE; i++) {
		fread(buf, 1, 3, gi->GeoIPDatabase);
		if (buf[0] == 255 && buf[1] == 255 && buf[2] == 255) {
			hasStructureInfo = 1;
			break;
		}
	}
	if (hasStructureInfo == 1) {
		fseek(gi->GeoIPDatabase, -3l, SEEK_CUR);
	} else {
		/* no structure info, must be pre Sep 2002 database, go back to end */
		fseek(gi->GeoIPDatabase, -3l, SEEK_END);
	}

	for (i = 0; i < DATABASE_INFO_MAX_SIZE; i++) {
		fread(buf, 1, 3, gi->GeoIPDatabase);
		if (buf[0] == 0 && buf[1] == 0 && buf[2] == 0) {
			retval = malloc(sizeof(char) * (i+1));
			if (retval == NULL) {
				return NULL;
			}
			fread(retval, 1, i, gi->GeoIPDatabase);
			retval[i] = '\0';
			return retval;
		}
		fseek(gi->GeoIPDatabase, -4l, SEEK_CUR);
	}
	return NULL;
}

/* GeoIP Region Edition functions */
GeoIPRegion * _get_region(GeoIP* gi, unsigned long ipnum) {
	GeoIPRegion * region;
	unsigned int seek_region;

	region = malloc(sizeof(GeoIPRegion));
	memset(region, 0, sizeof(GeoIPRegion));

	seek_region = _seek_country(gi, ipnum) - STATE_BEGIN;
	if (seek_region >= 1000) {
		region->country_code[0] = 'U';
		region->country_code[1] = 'S';
		region->country_code[2] = '\0';
		region->region = malloc(sizeof(char) * 3);
		region->region[0] = (char) ((seek_region - 1000)/26 + 65);
		region->region[1] = (char) ((seek_region - 1000)%26 + 65);
		region->region[2] = '\0';
	} else {
		strncpy(region->country_code, GeoIP_country_code[seek_region], 3);
		region->region = NULL;
	}
	return region;
}

GeoIPRegion * GeoIP_region_by_addr (GeoIP* gi, const char *addr) {
	unsigned long ipnum;
	if (addr == NULL) {
		return 0;
	}
	ipnum = _addr_to_num(addr);
	return _get_region(gi, ipnum);
}

GeoIPRegion * GeoIP_region_by_name (GeoIP* gi, const char *name) {
	unsigned long ipnum;
	struct hostent * host;
	if (name == NULL) {
		return 0;
	}
	ipnum = _addr_to_num(name);
	if (ipnum == 0) {
		host = gethostbyname(name);
		if (host == NULL) {
			return 0;
		}
		ipnum = _h_addr_to_num((unsigned char *) host->h_addr_list[0]);
	}
	return _get_region(gi, ipnum);
}

void GeoIPRegion_delete (GeoIPRegion *gir) {
	if (gir->region != NULL)
			free(gir->region);
	free(gir);
}

/* GeoIP Organization Edition functions */
char *_get_org (GeoIP* gi, unsigned long ipnum) {
	int seek_org;
	char buf[MAX_ORG_RECORD_LENGTH];
	char * org_buf, * buf_pointer;
	int record_pointer;
	seek_org = _seek_country(gi, ipnum);
	if (seek_org == gi->databaseSegments[0])		
		return NULL;

	record_pointer = seek_org + (2 * gi->record_length - 1) * gi->databaseSegments[0];

	if (gi->cache == NULL) {
		fseek(gi->GeoIPDatabase, record_pointer, SEEK_SET);
		fread(buf, sizeof(char), MAX_ORG_RECORD_LENGTH, gi->GeoIPDatabase);
		org_buf = malloc(sizeof(char) * (strlen(buf)+1));
		strcpy(org_buf, buf);
	} else {
		buf_pointer = gi->cache + (long)record_pointer;
		org_buf = malloc(sizeof(char) * (strlen(buf_pointer)+1));
		strcpy(org_buf, buf_pointer);
	}
	return org_buf;
}

char *GeoIP_org_by_addr (GeoIP* gi, const char *addr) {
	unsigned long ipnum;
	if (addr == NULL) {
		return 0;
	}
	ipnum = _addr_to_num(addr);
	return _get_org(gi, ipnum);
}

char *GeoIP_org_by_name (GeoIP* gi, const char *name) {
	unsigned long ipnum;
	struct hostent * host;
	if (name == NULL) {
		return 0;
	}
	ipnum = _addr_to_num(name);
	if (ipnum == 0) {
		host = gethostbyname(name);
		if (host == NULL) {
			return 0;
		}
		ipnum = _h_addr_to_num((unsigned char *) host->h_addr_list[0]);
	}
	return _get_org(gi, ipnum);
}

unsigned char GeoIP_database_edition (GeoIP* gi) {
	return gi->databaseType;
}
