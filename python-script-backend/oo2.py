import os
import azure.cognitiveservices.speech as speechsdk
from langdetect import detect, DetectorFactory
from langdetect.lang_detect_exception import LangDetectException
DetectorFactory.seed = 0

def identify_language(text):
    try:
        lang_code = detect(text)
        return lang_code
    except LangDetectException as e:
        print(f"Error: {e}")
        return None

# Example usage:

full_names = {
    'af': 'Afrikaans',
    'ar': 'Arabic',
    'bg': 'Bulgarian',
    'bn': 'Bengali',
    'ca': 'Catalan',
    'cs': 'Czech',
    'da': 'Danish',
    'de': 'German',
    'el': 'Greek',
    'en': 'English',
    'es': 'Spanish',
    'et': 'Estonian',
    'fa': 'Persian',
    'fi': 'Finnish',
    'fr': 'French',
    'gu': 'Gujarati',
    'he': 'Hebrew',
    'hi': 'Hindi',
    'hr': 'Croatian',
    'hu': 'Hungarian',
    'id': 'Indonesian',
    'it': 'Italian',
    'ja': 'Japanese',
    'kn': 'Kannada',
    'ko': 'Korean',
    'lt': 'Lithuanian',
    'lv': 'Latvian',
    'mk': 'Macedonian',
    'ml': 'Malayalam',
    'mr': 'Marathi',
    'ne': 'Nepali',
    'nl': 'Dutch',
    'no': 'Norwegian',
    'pa': 'Punjabi',
    'pl': 'Polish',
    'pt': 'Portuguese',
    'ro': 'Romanian',
    'ru': 'Russian',
    'sk': 'Slovak',
    'sl': 'Slovene',
    'so': 'Somali',
    'sq': 'Albanian',
    'sv': 'Swedish',
    'sw': 'Swahili',
    'ta': 'Tamil',
    'te': 'Telugu',
    'th': 'Thai',
    'tl': 'Tagalog',
    'tr': 'Turkish',
    'uk': 'Ukrainian',
    'ur': 'Urdu',
    'vi': 'Vietnamese',
    'zh-cn': 'Chinese (Simplified)',
    'zh-tw': 'Chinese (Traditional)'
}

def text_to_speech(text, language):
    # Replace with your subscription key and region
    subscription_key = 'FwIBSM6GcZVT2gtIq2qZx5NLrXO30qtEPptCgCFbKXeyCzn82qwnJQQJ99AKACYeBjFXJ3w3AAAYACOGp0H3'
    region = 'eastus'  # For example, 'eastus'

    # Create the speech config
    speech_config = speechsdk.SpeechConfig(subscription=subscription_key, region=region)

    # Set the speech synthesis language
    lang_code = language_code(language)
    speech_config.speech_synthesis_language = lang_code

    # Create a speech synthesizer object
    speech_synthesizer = speechsdk.SpeechSynthesizer(speech_config=speech_config)

    # Synthesize the text to speech
    result = speech_synthesizer.speak_text_async(text).get()

    if result.reason == speechsdk.ResultReason.SynthesizingAudioCompleted:
        print(f'Successfully synthesized the speech for: {text}')
    elif result.reason == speechsdk.ResultReason.Canceled:
        cancellation_details = result.cancellation_details
        print(f'Speech synthesis canceled: {cancellation_details.reason}')

def language_code(language):
    # Language code mapping for Azure
    lang_codes = {
    'afrikaans': 'af',
    'albanian': 'sq',
    'amharic': 'am',
    'arabic': 'ar',
    'armenian': 'hy',
    'assamese': 'as',
    'azerbaijani': 'az',
    'bangla': 'bn',
    'bosnian (latin)': 'bs',
    'bulgarian': 'bg',
    'cantonese (traditional)': 'yue',
    'catalan': 'ca',
    'chinese (literary)': 'lzh',
    'chinese simplified': 'zh-cn',
    'chinese traditional': 'zh-Hant',
    'croatian': 'hr',
    'czech': 'cs',
    'danish': 'da',
    'dari': 'prs',
    'dutch': 'nl',
    'english': 'en',
    'estonian': 'et',
    'fijian': 'fj',
    'filipino': 'fil',
    'finnish': 'fi',
    'french': 'fr',
    'french (canada)': 'fr-ca',
    'german': 'de',
    'greek': 'el',
    'gujarati': 'gu',
    'haitian creole': 'ht',
    'hebrew': 'he',
    'hindi': 'hi-IN',
    'hmong daw': 'mww',
    'hungarian': 'hu',
    'icelandic': 'is',
    'indonesian': 'id',
    'inuktitut': 'iu',
    'irish': 'ga',
    'italian': 'it',
    'japanese': 'ja',
    'kannada': 'kn-IN',
    'kazakh': 'kk',
    'khmer': 'km',
    'klingon': 'tlh-Latn',
    'klingon (plqad)': 'tlh-Piqd',
    'korean': 'ko',
    'kurdish (central)': 'ku',
    'kurdish (northern)': 'kmr',
    'lao': 'lo',
    'latvian': 'lv',
    'lithuanian': 'lt',
    'malagasy': 'mg',
    'malay': 'ms',
    'malayalam': 'ml-IN',
    'maltese': 'mt',
    'maori': 'mi',
    'marathi': 'mr-IN',
    'myanmar': 'my',
    'nepali': 'ne',
    'norwegian': 'nb',
    'odia': 'or',
    'pashto': 'ps',
    'persian': 'fa',
    'polish': 'pl',
    'portuguese (brazil)': 'pt',
    'portuguese (portugal)': 'pt-pt',
    'punjabi': 'pa',
    'queretaro otomi': 'otq',
    'romanian': 'ro',
    'russian': 'ru',
    'samoan': 'sm',
    'serbian (cyrillic)': 'sr-Cyrl',
    'serbian (latin)': 'sr-Latn',
    'slovak': 'sk',
    'slovenian': 'sl',
    'spanish': 'es',
    'swahili': 'sw',
    'swedish': 'sv',
    'tahitian': 'ty',
    'tamil': 'ta-IN',
    'telugu': 'te-IN',
    'thai': 'th',
    'tigrinya': 'ti',
    'tongan': 'to',
    'turkish': 'tr',
    'ukrainian': 'uk',
    'urdu': 'ur',
    'vietnamese': 'vi',
    'welsh': 'cy',
    'yucatec maya': 'yua'
}

    return lang_codes.get(language.lower(), 'en-US')  # Default to English if language not found

if __name__ == '__main__':
    text = "ಹೆಚ್ಚಾಗಿ ಬಳಸುವ ಕೆಲವು ಕನ್ನಡ ವಾಕ್ಯಗಳು ಇಲ್ಲಿವೆ:" 
    language = identify_language(text)
    if language:
            if language in full_names:
                language=full_names[language]
                print("The language is:",language)
     # Replace with the text you want to convert
    #language = "kannada"  # Replace with the target language

    text_to_speech(text, language)