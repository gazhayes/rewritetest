#!/usr/bin/env python3
# Copyright (c) 2015-2018 The Bitcoin Core developers
# Distributed under the MIT software license, see the accompanying
# file COPYING or http://www.opensource.org/licenses/mit-license.php.
"""Test share/rpcauth/rpcauth.py
"""
import base64
import configparser
import hmac
import importlib
import os
import sys
import unittest

class TestRPCAuth(unittest.TestCase):
    def setUp(self):
        config = configparser.ConfigParser()
        config_path = os.path.abspath(
            os.path.join(os.sep, os.path.abspath(os.path.dirname(__file__)),
            "../config.ini"))
        with open(config_path) as config_file:
            config.read_file(config_file)
        sys.path.insert(0, os.path.dirname(config['environment']['RPCAUTH']))
        self.rpcauth = importlib.import_module('rpcauth')

    def test_generate_salt(self):
        self.assertLessEqual(len(self.rpcauth.generate_salt()), 32)
        self.assertGreaterEqual(len(self.rpcauth.generate_salt()), 16)

    def test_generate_password(self):
        salt = self.rpcauth.generate_salt()
        password, password_hmac = self.rpcauth.generate_password(salt)

        expected_password = base64.urlsafe_b64encode(
            base64.urlsafe_b64decode(password)).decode('utf-8')
        self.assertEqual(expected_password, password)

    def test_check_password_hmac(self):
        salt = self.rpcauth.generate_salt()
        password, password_hmac = self.rpcauth.generate_password(salt)

        m = hmac.new(bytearray(salt, 'utf-8'),
            bytearray(password, 'utf-8'), 'SHA256')
        expected_password_hmac = m.hexdigest()

        self.assertEqual(expected_password_hmac, password_hmac)

if __name__ == '__main__':
    unittest.main()
