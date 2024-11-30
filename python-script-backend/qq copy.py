import firebase_admin
from firebase_admin import credentials, firestore
import re
from oo2 import text_to_speech
from datetime import datetime, timedelta
import time

# Replace with your Firebase project credentials file path
cred = credentials.Certificate('devv3-83faf-firebase-adminsdk-jo0wc-10a36ea28a.json')

# Initialize Firebase Admin SDK
firebase_admin.initialize_app(cred)

# Initialize Firestore client
db = firestore.client()

def get_latest_text_field_values(collection_name, processed_doc_ids, script_start_time, last_doc_snapshot=None):
    # Reference to the collection
    collection_ref = db.collection(collection_name)

    # Query to fetch documents after a specific document (pagination)
    query = collection_ref.order_by('timestamp', direction='ASCENDING')
    if last_doc_snapshot:
        query = query.start_after(last_doc_snapshot)

    # Get the documents from Firestore
    docs = query.limit(10).get()  # Limit to 10 documents at a time

    if not docs:
        return processed_doc_ids, None  # No new documents to process, return early

    last_doc_snapshot = docs[-1]  # Get the last document for pagination

    # Process each document
    for doc in docs:
        doc_id = doc.id  # Document ID
        doc_data = doc.to_dict()

        # Skip if this document has already been processed
        if doc_id in processed_doc_ids:
            continue

        # Retrieve the timestamp field (stored as a string in Firestore)
        timestamp_str = doc_data.get('timestamp')
        text_value = doc_data.get('name')

        # Skip if timestamp or name field is N/A or None
        if not timestamp_str or timestamp_str.strip().upper() == 'N/A' or not text_value or text_value.strip().upper() == 'N/A':
            processed_doc_ids.add(doc_id)  # Mark it as processed to avoid re-checking
            continue

        try:
            if isinstance(timestamp_str, str):
                # Use regex to match and remove 'at' and UTC+<timezone>
                timestamp_str = re.sub(r" at|\sUTC[^\s]*", "", timestamp_str)

                # Convert the timestamp string to a datetime object (ignore timezone first)
                timestamp = datetime.strptime(timestamp_str, "%B %d, %Y %I:%M:%S %p")

                # Skip if the document was created before the script start time
                if timestamp < script_start_time:
                    processed_doc_ids.add(doc_id)  # Mark it as processed to avoid re-checking
                    continue

                print(f"The Translated text is: {text_value}")

                # Convert text to speech
                langg = 'en'
                text_to_speech(text_value, langg)

                # Print the 'text' field value if it exists and is not empty
                if isinstance(text_value, str) and text_value.strip() != "":
                    print(f'Text Field Value: {text_value}')

                print('-------------------------')

                # Mark this document ID as processed
                processed_doc_ids.add(doc_id)

            else:
                print(f"Invalid timestamp format for document {doc_id}. Skipping this document.")
                continue  # Skip if timestamp is not a valid string

        except ValueError as e:
            print(f"Error parsing timestamp for document ID {doc_id}: {timestamp_str}. Error: {e}")
            continue  # Skip this document if timestamp parsing fails

    return processed_doc_ids, last_doc_snapshot

def main():
    collection_name = 'zed'  # Replace with your collection name

    # Record the script's start time
    script_start_time = datetime.now()

    print(f"Script started at: {script_start_time}")

    # Initialize set to store processed document IDs
    processed_doc_ids = set()
    last_doc_snapshot = None  # Used for pagination

    while True:
        try:
            # Retrieve the most recent added documents and speak the 'name' field
            processed_doc_ids, last_doc_snapshot = get_latest_text_field_values(
                collection_name, processed_doc_ids, script_start_time, last_doc_snapshot
            )

            # Sleep for a specified interval (e.g., 60 seconds) before querying again
            # time.sleep(60)

        except KeyboardInterrupt:
            print("Process interrupted by the user.")
            break

if __name__ == '__main__':
    main()