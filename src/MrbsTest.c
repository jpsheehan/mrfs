#include <stdio.h>
#include <stdlib.h>

#include "Mrbs.h"
#include ".env.h"

#define B_YEAR 2020
#define B_MONTH 8
#define B_DAY 9
#define B_HOUR 10
#define B_MINUTE 30

#define B_ROOM MRBS_ROOM_212A
#define B_AREA MRBS_AREA_EPS

#define B_NAME "Test MRFS"
#define B_DESCRIPTION "TEST"

int main()
{
	mrbs_credentials_t c = { .username = USERNAME, .password = PASSWORD };
	mrbs_booking_t b = { 0 };

	if (MrbsBookingCreate(&b, &c, B_NAME, B_DESCRIPTION, B_YEAR, B_MONTH, B_DAY, B_HOUR, B_MINUTE, MRBS_DUR_30_MINS, B_AREA, B_ROOM) == 0) {
		printf("Created booking successful.\n");

		if (MrbsBookingSend(&b) == 0)
		{
			printf("Booking sent!\n");
		}
		else
		{
			printf("Booking not send :(\n");
		}

	} else {
		printf("Failed to create booking.\n");
	}

	// reset the information for the booking
	// with the minimum amount of information needed to get the booking from the system
	b = (mrbs_booking_t){ 0 };
	if (MrbsBookingGet(&b, &c, B_YEAR, B_MONTH, B_DAY, B_HOUR, B_MINUTE, B_ROOM) == 0)
	{
		printf("Got the booking! Name: \"%s\", Description: \"%s\"\n", b.name, b.description);
	}
	else
	{
		printf("Ran into a problem with getting the booking.\n");
		return -1;
	}

	// maybe delete the booking if the user wants to
	char r = 'n';
	printf("Do you want to delete the booking? (y/n)\n");
	scanf("%c", &r);

	if (r == 'y')
	{
		printf("\nDeleting booking\n");
	
		if ( MrbsBookingDelete(&b) == 0)
		{
			printf("Booking %d deleted!\n", b._id);
		}
		else
		{
			printf("Could not delete booking :(\n");
			return -1;
		}

	}
	else
	{
		printf("\nNot deleting booking\n");
	}
	
	MrbsBookingDestroy(&b);

	return EXIT_SUCCESS;
}
