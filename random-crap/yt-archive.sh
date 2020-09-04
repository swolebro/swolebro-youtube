#!/bin/bash

# Example script for downloading a YouTube channel,
# with the ability to incremental updates.

# (c) The Swolesoft Development Group, 2020
# License: http://250bpm.com/blog:82

# Any videos downloaded are copyright of their prospective holders.
# So you've got personal use and fair use, but commercial use and
# redistribution are a no-go unless you're backing up your own channel.

#set -eux # french debug mode

if [ "$#" -ne 1 ]; then
    echo "usage: ./yt-archive.sh <token>"
    echo
    echo "The ways in which you can specify that token are weird and inconsistent."
    echo "channel/<name>, c/<name>, user/<name>, etc., where sometimes a given"
    echo "name works with one prefix but not the others, and sometimes it's"
    echo "gibberish, or if they have a special channel name, it's not, etc. etc."
    echo "Try a few, you'll figure it out."
    exit 1
fi

channel="https://www.youtube.com/${1}"


# Download the json for every video published, either from the very start, or
# from the last time you ran this. Save to file and spit to stdout. Note that
# the channel playlist is naturally reverse chrono order, so we reverse *that*
# on the first download, so the newest stuff is at the bottom, then do a bit
# of bullshit to keep appending to it.

if [ ! -e channel.json ]; then
    youtube-dl  --playlist-reverse --ignore-errors \
        --dump-json "${channel}"  2>error.log | jq -c '.' > channel.json
else

    # This file will track the latest additions.
    truncate --size 0 channel.new

    # Get the latest id seen.
    old_id=$(tail -n 1 channel.json | jq -r '.id')
    old_date=$(tail -n 1 channel.json | jq -r '.upload_date')

    # The --dateafter option is no good, because you have to pull down the full
    # channel JSON each time. Going by .n_entries or .id alone could get fucked
    # when videos get removed. So, I guess we'll just paginate in groups of 10,
    # since that keeps it reasonably quick for updating.

    i=1
    while true; do

        # Save how many lines this pagination returned, in case we scan past
        # our date mark looking for a missing id.
        added=$(youtube-dl --dateafter ${old_date} --ignore-errors --dump-json \
            --playlist-start ${i} --playlist-end $((i+9)) "${channel}"  \
            2>>error.log | tee -a channel.new | wc -l)

        # Rescan the whole channel.new looking to see if we've found the old_id.
        jq '.id' channel.new | grep -m1 -qF -- "$old_id"
        rc=$?

        if [ $added -eq 0 ] || [ $rc -eq 0 ]; then
            # Print up until the repeat id (if found, or all if date-limited), flip, append.
            # Could cause some duplication in rare edge cases I'm too lazy to fuck with.
            jq -c --arg old "${old_id}" 'if .id!=$old then . else empty end' channel.new | tac >> channel.json
            break
        fi

        i=$((i+10))
    done
fi


# Now we're going to go through, double check for every id we're missing video
# for, then download that. Could do it just based on the the new file, but I
# think this will be more resilient against restarts and shit.

jq -r .id channel.json | sort > available.ids

# Magic to extract ids already downloaded.
find -type f |
    grep -P '.(mkv|webm|mp4)$' |
    rev |
    cut -d. -f 2- |
    cut -c -11 |
    rev |
    sort > found.ids

comm -23 available.ids found.ids > download.ids

cat download.ids | while read line; do
    # The 247 and 302 are 720p vp9 encoded at 30fps and 60fps, respectively, which appears to be a common
    # one available, with good quality and not an obnoxiously large fileszie. For those where you can't get
    # it, just default to whatever is best under 720p. Audio, always take the best.

    # To try other formats, look to .formats[].format_id in the channel.json,
    # and read the youtube-dl docs. It's possible to get just the audio as well
    # if using --extract-audio and --format 'bestaudio/best'

    youtube-dl --limit-rate 6.5M \
        --write-auto-sub --sub-lang en \
        --format '(247/302/bestvideo[height<=720])+bestaudio/best' -- "${line}" \
        2>>error.log | tee download.log
done


# Possible improvements:
# * make the format or download rate limit configurable
# * or have a flag for this to just generate the id list, and you write whatever
#   while loop you want
# * make the channel JSON into an easy to read/grep index
# * sort stuff into folders by date, episode number, or other
# * make something useful out of the autosub vtt's
# * retry downloading subs if they weren't available before
# * do some sort of post-check for files that might not have downloaded
#   well (based on `file` output, size, or playability)
# * something smart so you don't continually try to redownload shit that's really not available
# * real logging
