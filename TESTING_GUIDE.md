# IRC Server Testing Guide

## Quick Automated Tests

To verify all bug fixes are working:

```bash
# 1. Compile the server
make clean && make

# 2. Start the server
./ircserv 6667 testpass &

# 3. Run individual tests
```

## Manual Test Cases

### Test 1: Authentication Flow
```bash
# Connect with wrong password
echo "PASS wrongpass" | nc localhost 6667
# Expected: "Error: Incorrect Password"

# Connect with correct password
echo -e "PASS testpass" | nc localhost 6667
# Expected: "Please Enter Your Nickname"
```

### Test 2: Nickname Validation (IRC Error Codes)
```bash
# Test empty nickname (should return IRC 431)
{ echo "PASS testpass"; sleep 0.5; echo "NICK "; } | nc localhost 6667
# Expected: "431" error code

# Test invalid nickname starting with number (should return IRC 432)
{ echo "PASS testpass"; sleep 0.5; echo "NICK 123test"; } | nc localhost 6667
# Expected: "432" error code

# Test too long nickname (should return IRC 432)
{ echo "PASS testpass"; sleep 0.5; echo "NICK verylongnicknamethatiswayoverthirtychars"; } | nc localhost 6667
# Expected: "432" error code
```

### Test 3: Successful Authentication
```bash
{ echo "PASS testpass"; sleep 0.5; echo "NICK testuser"; sleep 0.5; echo "USER test 0 * :Test User"; sleep 2; } | nc localhost 6667
# Expected: Welcome messages with IRC 001-004
```

### Test 4: Channel Operations

#### Create and Join Channel
```bash
{ echo "PASS testpass"; sleep 0.5; echo "NICK user1"; sleep 0.5; echo "USER u1 0 * :User1"; sleep 1; echo "JOIN #testchan"; sleep 2; } | nc localhost 6667
# Expected: JOIN message, MODE +nt, NAMES list, RPL_ENDOFNAMES
```

#### Test MODE on Non-existent Channel (IRC 403)
```bash
{ echo "PASS testpass"; sleep 0.5; echo "NICK user2"; sleep 0.5; echo "USER u2 0 * :User2"; sleep 1; echo "MODE #nonexist +i"; sleep 1; } | nc localhost 6667
# Expected: "403" No such channel error
```

### Test 5: Invite-Only Channel (IRC 473)

Terminal 1 - Create invite-only channel:
```bash
{ echo "PASS testpass"; echo "NICK chanop"; echo "USER cop 0 * :Op"; echo "JOIN #invite"; echo "MODE #invite +i"; cat; } | nc localhost 6667
```

Terminal 2 - Try to join without invite:
```bash
{ echo "PASS testpass"; sleep 0.5; echo "NICK regular"; sleep 0.5; echo "USER reg 0 * :Reg"; sleep 1; echo "JOIN #invite"; sleep 2; } | nc localhost 6667
# Expected: "473" Cannot join channel (+i)
```

### Test 6: Channel Limits (IRC 471)

Terminal 1 - Create channel with limit:
```bash
{ echo "PASS testpass"; echo "NICK op"; echo "USER o 0 * :O"; echo "JOIN #limited"; echo "MODE #limited +l 2"; cat; } | nc localhost 6667
```

Terminal 2 & 3 - Fill the channel:
```bash
{ echo "PASS testpass"; echo "NICK user1"; echo "USER u1 0 * :U1"; echo "JOIN #limited"; cat; } | nc localhost 6667
{ echo "PASS testpass"; echo "NICK user2"; echo "USER u2 0 * :U2"; echo "JOIN #limited"; cat; } | nc localhost 6667
```

Terminal 4 - Try to exceed limit:
```bash
{ echo "PASS testpass"; sleep 0.5; echo "NICK user3"; sleep 0.5; echo "USER u3 0 * :U3"; sleep 1; echo "JOIN #limited"; sleep 2; } | nc localhost 6667
# Expected: "471" Cannot join channel (+l)
```

### Test 7: Password-Protected Channel (IRC 475)

Terminal 1 - Create channel with password:
```bash
{ echo "PASS testpass"; echo "NICK op"; echo "USER o 0 * :O"; echo "JOIN #private"; echo "MODE #private +k secret123"; cat; } | nc localhost 6667
```

Terminal 2 - Try to join with wrong password:
```bash
{ echo "PASS testpass"; sleep 0.5; echo "NICK user"; sleep 0.5; echo "USER u 0 * :U"; sleep 1; echo "JOIN #private wrongpass"; sleep 2; } | nc localhost 6667
# Expected: "475" Cannot join channel (+k)
```

Terminal 3 - Join with correct password:
```bash
{ echo "PASS testpass"; sleep 0.5; echo "NICK user2"; sleep 0.5; echo "USER u2 0 * :U2"; sleep 1; echo "JOIN #private secret123"; sleep 2; } | nc localhost 6667
# Expected: Successfully joins
```

### Test 8: Operator Privileges (IRC 482)

Terminal 1 - Channel operator:
```bash
{ echo "PASS testpass"; echo "NICK op"; echo "USER o 0 * :O"; echo "JOIN #chan"; cat; } | nc localhost 6667
```

Terminal 2 - Regular user tries MODE:
```bash
{ echo "PASS testpass"; sleep 0.5; echo "NICK regular"; sleep 0.5; echo "USER r 0 * :R"; sleep 1; echo "JOIN #chan"; sleep 1; echo "MODE #chan +t"; sleep 2; } | nc localhost 6667
# Expected: "482" You're not channel operator
```

### Test 9: KICK Command

Terminal 1 - Operator:
```bash
{ echo "PASS testpass"; echo "NICK op"; echo "USER o 0 * :O"; echo "JOIN #kick"; cat; } | nc localhost 6667
```

Terminal 2 - User to be kicked:
```bash
{ echo "PASS testpass"; echo "NICK victim"; echo "USER v 0 * :V"; echo "JOIN #kick"; cat; } | nc localhost 6667
```

In Terminal 1, type:
```
KICK #kick victim :You are out
```
Expected: victim receives KICK message and is removed from channel
Also verify operator status is removed if kicked user was an op

### Test 10: TOPIC Command

Terminal 1 - Create channel with +t:
```bash
{ echo "PASS testpass"; echo "NICK op"; echo "USER o 0 * :O"; echo "JOIN #topic"; cat; } | nc localhost 6667
```

Terminal 2 - Non-op tries to change topic:
```bash
{ echo "PASS testpass"; sleep 0.5; echo "NICK regular"; sleep 0.5; echo "USER r 0 * :R"; sleep 1; echo "JOIN #topic"; sleep 1; echo "TOPIC #topic :New Topic"; sleep 2; } | nc localhost 6667
# Expected: "482" You're not channel operator (since +t is set by default)
```

### Test 11: CRLF Line Endings

All messages should end with `\r\n` (CRLF), not just `\n` (LF):
```bash
{ echo "PASS testpass"; sleep 0.5; echo "NICK test"; sleep 0.5; echo "USER t 0 * :T"; sleep 2; } | nc localhost 6667 | od -An -tx1 | grep "0d 0a"
# Expected: Should find "0d 0a" (hex for \r\n)
```

## Test with irssi Client

The best way to test is with a real IRC client:

```bash
# Install irssi if not available
sudo apt-get install irssi

# Start your server
./ircserv 6667 testpass

# In another terminal, connect with irssi
irssi

# In irssi:
/connect localhost 6667 testpass
/nick myname
/join #test
/mode #test +i
```

Try all the scenarios above using irssi commands and verify:
- ✅ No white screens
- ✅ Error messages display properly
- ✅ Channel modes work correctly
- ✅ KICK, TOPIC, INVITE all function
- ✅ Proper error codes shown

## Expected Results Summary

All bug fixes should result in:
- ✅ Proper IRC numeric error codes (401-482 range)
- ✅ CRLF line endings (\r\n) on all messages
- ✅ No crashes on invalid input
- ✅ Channel existence checks prevent segfaults
- ✅ Kicked users lose all privileges (operator, invited)
- ✅ Non-operators cannot set channel modes
- ✅ Nickname validation enforces RFC standards
- ✅ Invite-only channels return IRC 473
- ✅ Full channels return IRC 471
- ✅ Wrong password returns IRC 475
- ✅ No duplicate user entries in channels
