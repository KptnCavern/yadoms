# yScriptApi documentation

## Constants
* kTimeout : constant returned by some methods in case of timeout occurs

## Types
### CWaitForEventResult
#### Description
Result type of a call to waitForEvent method
#### Members
* getType : Get the type of result (see EResultType)
* getKeywordId : Get the signaled keyword ID
* getValue : Get the value of the signaled event
#### Types
##### EResultType : Type of result
Values:
* kTimeout : timeout
* kKeyword : event on keyword
* kDateTime : datetime event

## Methods

### getKeywordId
#### Description
Retrieve the keyword ID from its device and keyword friendly names
#### Parameters
##### deviceName (in)
Device name containing the keyword
##### keywordName (in)
Keyword name to search for ID
#### Return
Found keyword ID
#### Throw
Error if several (some have to be renamed) or none keyword ID found

### getRecipientId
#### Description
Retrieve the recipient ID from its first and last names
#### Parameters
##### firstName (in)
First name
##### lastName (in)
Last name
#### Return
Found recipient ID
#### Throw
Error if no recipient found

### readKeyword
#### Description
Read the last known state of the keyword
#### Parameters
##### keywordId (in)
The keyword ID from which retrieve state
#### Return
The last known keyword state (empty if no known state)
#### Throw
Error if keyword not found

### waitForNextAcquisition
#### Description
Wait for a new acquisition on a keyword
#### Parameters
##### keywordId (in)
The keyword ID to watch
##### timeout (in)
Timeout to wait. Can be a duration (format \"hh:mm:ss\") or a dateTime (format \"YYYY-MM-DD hh:mm:ss\"). No timeout if empty (default).  
If dateTime is used and specified in the past, the function will immediately return.
#### Return
The keyword new state (empty if timeout)
#### Throw
Error if keyword not found

### waitForNextAcquisitions
#### Description
Wait for a new acquisition on a keyword list
#### Parameters
##### keywordIdList (in)
The keyword IDs list to watch
##### timeout (in)
Timeout to wait. Can be a duration (format \"hh:mm:ss\") or a dateTime (format \"YYYY-MM-DD hh:mm:ss\"). No timeout if empty (default).  
If dateTime is used and specified in the past, the function will immediately return.
#### Return
Returned value is a pair of the keyword Id who changed, and its new value. The keyword Id is kTimeout if timeout.
#### Throw
Error if at least one keyword was not found

### waitForEvent
#### Description
Wait for an event (acquisition, time event...)
#### Parameters
##### keywordIdList (in)
The keyword IDs list to watch (can be empty list to only wait for time events)
##### receiveDateTimeEvent (in)
True to receive date/time events (one per minute)
##### timeout (in)
Timeout to wait. Can be a duration (format \"hh:mm:ss\") or a dateTime (format \"YYYY-MM-DD hh:mm:ss\"). No timeout if empty (default).  
If dateTime is used and specified in the past, the function will immediately return.
#### Return
Returned value is a CWaitForEventResult instance, containing the event type and value.
#### Throw
Error if at least one keyword was not found

### writeKeyword
#### Description
Change state of a keyword.  
Do nothing if keyword is not found or not writable.
#### Parameters
##### keywordId (in)
The keyword ID to change state
##### newState (in)
The keyword new state. Must be a string (ex, for boolean, must be \"1\" or \"0\")
#### Throw
Error if keyword not found

### sendNotification
#### Description
Send a notification.  
Do nothing if keyword or recipient is not found or not writable.
#### Parameters
##### keywordId (in)
The keyword ID to use to send notification
##### recipientId (in)
The recipient ID
##### message (in)
The message to send
#### Throw
Error if keyword or recipient not found

### getInfo
#### Description
Get general information
#### Parameters
##### key (in)
Information key, from this list (don't forget to prefix by 'yApi.') :
* kSunrise : get current day sunrise time (format is 'HH:MM')
* kSunset : get current day sunset time (format is 'HH:MM')
* kLatitude : get latitude configured by user (or approximative latitude if not configured)
* kLongitude : get longitude configured by user (or approximative longitude if not configured)
* kAltitude : get altitude configured by user (or approximative altitude if not configured)
* kYadomsServerOS : returns a string containing the OS name where Yadoms server is running
* kYadomsServerVersion : returns a string containing the Yadoms server version
#### Return
Information as string (empty string if not found)
#### Throw
Error if key doesn't exist
